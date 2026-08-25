// ----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2018 Fons Adriaensen <fons@linuxaudio.org>
//    
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------


#ifndef _ZITA_CONVOLVER_H
#define _ZITA_CONVOLVER_H


#include <pthread.h>
#include <stdint.h>
#include <fftw3.h>
#include <mutex>
#include <condition_variable>
 
#define ZITA_CONVOLVER_MAJOR_VERSION 4
#define ZITA_CONVOLVER_MINOR_VERSION 0


extern int zita_convolver_major_version (void);
extern int zita_convolver_minor_version (void);


// ----------------------------------------------------------------------------


#ifdef ZCSEMA_IS_IMPLEMENTED
#undef ZCSEMA_IS_IMPLEMENTED
#endif

class ZCsema
{
public:

    ZCsema (void) : _count (0)
    {
        init (0, 0);
    }

    ~ZCsema (void) {}

    ZCsema (const ZCsema&); // disabled
    ZCsema& operator= (const ZCsema&); // disabled

    int init (int s, int v)
    {
	std::unique_lock<std::mutex> lk(_mutex);

	_count = v;
	return 0;
    }

    int post (void)
    {
	std::unique_lock<std::mutex> lk(_mutex);

	_count++;
	if (_count == 1) _cond.notify_one();
	return 0;
    }

    int wait (void)
    {
	std::unique_lock<std::mutex> lk(_mutex);

	while (_count < 1)  _cond.wait(lk);
	_count--;
	return 0;
    }

    int trywait (void)
    {
	if (! _mutex.try_lock()) return -1;
	if (_count < 1)
	{
	    _mutex.unlock();
	    return -1;
	}
        _count--;
	_mutex.unlock();
        return 0;
    }

private:

    int              _count;
    std::mutex       _mutex;
    std::condition_variable _cond;
};

#define ZCSEMA_IS_IMPLEMENTED


#ifndef ZCSEMA_IS_IMPLEMENTED
#error "The ZCsema class is not implemented."
#endif


// ----------------------------------------------------------------------------


class Inpnode   
{
private:

    friend class Convlevel;

    Inpnode (uint16_t inp);
    ~Inpnode (void);
    void alloc_ffta (uint16_t npar, int32_t size);
    void free_ffta (void);
    
    Inpnode        *_next;
    fftwf_complex **_ffta;
    uint16_t        _npar;
    uint16_t        _inp;
};


class Macnode  
{
private:

    friend class Convlevel;

    Macnode (Inpnode *inpn);
    ~Macnode (void);
    void alloc_fftb (uint16_t npar);
    void free_fftb (void);

    Macnode        *_next;
    Inpnode        *_inpn;
    Macnode        *_link;
    fftwf_complex **_fftb;
    uint16_t        _npar;
};


class Outnode   
{
private:

    friend class Convlevel;

    Outnode (uint16_t out, int32_t size);
    ~Outnode (void);
    
    Outnode        *_next;
    Macnode        *_list;
    float          *_buff [3];
    uint16_t        _out;
};


class Converror
{
public:

    enum
    {
	BAD_STATE = -1,
	BAD_PARAM = -2,
        MEM_ALLOC = -3
    };

    Converror (int error) : _error (error) {}

private:

    int _error;
};


class Convlevel
{
private:

    friend class Convproc;

    enum 
    {
        OPT_FFTW_MEASURE = 1,
        OPT_VECTOR_MODE  = 2,
        OPT_LATE_CONTIN  = 4
    };

    enum
    {
        ST_IDLE,
        ST_TERM,
        ST_PROC
    };

    static unsigned s_IdCounter;

    unsigned id;

    Convlevel (void);
    ~Convlevel (void);

    void configure (int      prio,
                    uint32_t offs,
                    uint32_t npar,
                    uint32_t parsize,
		    uint32_t options);

    void impdata_write (uint32_t  inp,
                        uint32_t  out,
                        int32_t   step,
                        float     *data,
                        int32_t   ind0,
                        int32_t   ind1,
                        bool      create);

    void impdata_clear (uint32_t  inp,
	                uint32_t  out);

    void impdata_link (uint32_t  inp1,
                       uint32_t  out1,
                       uint32_t  inp2,
                       uint32_t  out2);

    void reset (uint32_t  inpsize,
                uint32_t  outsize,
	        float     **inpbuff,
	        float     **outbuff);

    void start (int absprio, int policy);

    void process (bool sync);

    int  readout (bool sync, uint32_t skipcnt);

    void stop (void);

    void cleanup (void);

    void fftswap (fftwf_complex *p);

    void print (FILE *F);

    static void *static_main (void *arg);

    void main (void);

    Macnode *findmacnode (uint32_t inp, uint32_t out, bool create);


    volatile uint32_t   _stat;           // current processing state
    int                 _prio;           // relative priority
    uint32_t            _offs;           // offset from start of impulse response
    uint32_t            _npar;           // number of partitions
    uint32_t            _parsize;        // partition and outbut buffer size
    uint32_t            _outsize;        // step size for output buffer
    uint32_t            _outoffs;        // offset into output buffer
    uint32_t            _inpsize;        // size of shared input buffer 
    uint32_t            _inpoffs;        // offset into input buffer
    uint32_t            _options;        // various options
    uint32_t            _ptind;          // rotating partition index
    uint32_t            _opind;          // rotating output buffer index
    int                 _bits;           // bit identifiying this level
    int                 _wait;           // number of unfinished cycles
    pthread_t           _pthr;           // posix thread executing this level
    ZCsema              _trig;           // sema used to trigger a cycle
    ZCsema              _done;           // sema used to wait for a cycle
    Inpnode            *_inp_list;       // linked list of active inputs
    Outnode            *_out_list;       // linked list of active outputs
    fftwf_plan          _plan_r2c;       // FFTW plan, forward FFT
    fftwf_plan          _plan_c2r;       // FFTW plan, inverse FFT
    float              *_time_data;      // workspace
    float              *_prep_data;      // workspace
    fftwf_complex      *_freq_data;      // workspace
    float             **_inpbuff;        // array of shared input buffers
    float             **_outbuff;        // array of shared output buffers
};


// ----------------------------------------------------------------------------


class Convproc
{
public:

    Convproc (void);
    ~Convproc (void);

    enum
    {
        ST_IDLE,
	ST_STOP,
        ST_WAIT,
        ST_PROC
    };

    enum
    {
        FL_LATE = 0x0000FFFF,
	FL_LOAD = 0x01000000
    };

    enum 
    {
        OPT_FFTW_MEASURE = Convlevel::OPT_FFTW_MEASURE, 
        OPT_VECTOR_MODE  = Convlevel::OPT_VECTOR_MODE,
        OPT_LATE_CONTIN  = Convlevel::OPT_LATE_CONTIN
    };

    enum
    {
	MAXINP   = 64,
	MAXOUT   = 64,
	MAXLEV   = 8,
	MINPART  = 64,
	MAXPART  = 8192,
	MAXDIVIS = 16,
	MINQUANT = 16,
	MAXQUANT = 8192
    };

    uint32_t state (void) const
    {
	return _state;
    }

    float *inpdata (uint32_t inp) const
    {
	return _inpbuff [inp] + _inpoffs;
    }

    float *outdata (uint32_t out) const
    {
	return _outbuff [out] + _outoffs;
    }

    int configure (uint32_t  ninp,
                   uint32_t  nout,
                   uint32_t  maxsize,
                   uint32_t  quantum,
                   uint32_t  minpart,
                   uint32_t  maxpart,
		   float     density);

    int impdata_create (uint32_t  inp,
                        uint32_t  out,
                        int32_t   step,
                        float     *data,
                        int32_t   ind0,
                        int32_t   ind1); 

    int impdata_clear (uint32_t  inp,
	               uint32_t  out);

    int impdata_update (uint32_t  inp,
                        uint32_t  out,
                        int32_t   step,
                        float     *data,
                        int32_t   ind0,
                        int32_t   ind1); 

    int impdata_link (uint32_t  inp1,
                      uint32_t  out1,
                      uint32_t  inp2,
                      uint32_t  out2);

    // Deprecated, use impdata_link() instead.
    int impdata_copy (uint32_t  inp1,
                      uint32_t  out1,
                      uint32_t  inp2,
                      uint32_t  out2)
    {
	return impdata_link (inp1, out1, inp2, out2);
    }
    
    void set_options (uint32_t options);

    void set_skipcnt (uint32_t skipcnt);

    int  reset (void);

    int  start_process (int abspri, int policy);

    int  process (bool sync = false);

    int  stop_process (void);

    bool check_stop (void);

    int  cleanup (void);

    void print (FILE *F = stdout);

private:

    uint32_t    _state;                   // current state
    float      *_inpbuff [MAXINP];        // input buffers
    float      *_outbuff [MAXOUT];        // output buffers
    uint32_t    _inpoffs;                 // current offset in input buffers
    uint32_t    _outoffs;                 // current offset in output buffers
    uint32_t    _options;                 // option bits
    uint32_t    _skipcnt;                 // number of frames to skip 
    uint32_t    _ninp;                    // number of inputs
    uint32_t    _nout;                    // number of outputs
    uint32_t    _quantum;                 // processing block size
    uint32_t    _minpart;                 // smallest partition size
    uint32_t    _maxpart;                 // largest allowed partition size
    uint32_t    _nlevels;                 // number of partition sizes
    uint32_t    _inpsize;                 // size of input buffers
    uint32_t    _latecnt;                 // count of cycles ending too late
    Convlevel  *_convlev [MAXLEV];        // array of processors 
    void       *_dummy [64];

    static float  _mac_cost;
    static float  _fft_cost;
};


// ----------------------------------------------------------------------------


#endif

