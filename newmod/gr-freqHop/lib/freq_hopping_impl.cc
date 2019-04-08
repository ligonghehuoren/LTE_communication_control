/* -*- c++ -*- */
/* 
 * Copyright 2018 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "freq_hopping_impl.h"
#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace std;    
namespace gr {
  namespace freqHop {

    freq_hopping::sptr
    freq_hopping::make(float start_freq, float center_freq_one,float center_freq_two,float center_freq_three, float freq_delta,float period_ms,float end_freq)
    {
      return gnuradio::get_initial_sptr
        (new freq_hopping_impl(start_freq, center_freq_one, center_freq_two,center_freq_three,freq_delta, period_ms,end_freq));
    }

    freq_hopping_impl::freq_hopping_impl(float start_freq, float center_freq_one,float center_freq_two, float center_freq_three, float freq_delta,float period_ms,float end_freq)
      : block("freq_hopping",
              io_signature::make(0, 0, 0),
              io_signature::make(0, 0, 0)),
        d_finished(false),
	d_start_freq(start_freq),
	d_center_freq_one(center_freq_one),
	d_center_freq_two(center_freq_two),
	d_center_freq_three(center_freq_three),
	//d_nchannels(nchannels),
	d_freq_delta(freq_delta),
        d_period_ms(period_ms),
        d_end_freq(end_freq),
        //d_nchannels = (d_end_freq-d_start_freq)/d_freq_delta-2,
        d_port(pmt::mp("strobe"))
    {
      message_port_register_out(d_port);

      message_port_register_in(pmt::mp("set_msg"));
      set_msg_handler(pmt::mp("set_msg"),
                      boost::bind(&freq_hopping_impl::set_msg, this, _1));
    }

    freq_hopping_impl::~freq_hopping_impl()
    {
    }

    bool
    freq_hopping_impl::start()
    {
      // NOTE: d_finished should be something explicitly thread safe. But since
      // nothing breaks on concurrent access, I'll just leave it as bool.
      d_finished = false;
      d_thread = boost::shared_ptr<gr::thread::thread>
        (new gr::thread::thread(boost::bind(&freq_hopping_impl::run, this)));

      return block::start();
    }

    bool
    freq_hopping_impl::stop()
    {
      // Shut down the thread
      d_finished = true;
      d_thread->interrupt();
      d_thread->join();

      return block::stop();
    }

    void freq_hopping_impl::run()
    {

      int cir_cnt = 0,t=0;
      pmt::pmt_t msg;
      float next_freq,present_freq;
      pmt::pmt_t str0 = pmt::intern(std::string("freq"));
      pmt::pmt_t freq;

      while(!d_finished) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(d_period_ms));
        if(d_finished) {
          return;
        }
	if(d_start_freq!=0)
	{
	while(cir_cnt == ((d_end_freq-d_start_freq)/d_freq_delta+1))
		cir_cnt = 0;

	next_freq = cir_cnt * d_freq_delta + d_start_freq;
        cout<<next_freq<<endl;
	freq = pmt::from_float(next_freq);
        
        /*if(next_freq > d_end_freq)
        break;*/
	msg = pmt::cons(str0,freq);
        message_port_pub(d_port, msg);
	cir_cnt++;
	}
	else
	{
			if(t==0&&d_center_freq_one!=0)
			{
				cout<<d_center_freq_one<<endl;
				//present_freq=d_center_freq_one;
				freq = pmt::from_float(d_center_freq_one);
				msg = pmt::cons(str0,freq);
				message_port_pub(d_port, msg);
			}
			else if(t==1&&d_center_freq_two!=0)
			{
				cout<<d_center_freq_two<<endl;
				//present_freq=d_center_freq_one;
				freq = pmt::from_float(d_center_freq_two);
				msg = pmt::cons(str0,freq);
				message_port_pub(d_port, msg);
			}
			else if(t==2&&d_center_freq_three!=0)
			{
				cout<<d_center_freq_three<<endl;
				//present_freq=d_center_freq_one;
				freq = pmt::from_float(d_center_freq_three);
				msg = pmt::cons(str0,freq);
				message_port_pub(d_port, msg);
			}
			/*else
			d_finished = true;*/
			t++;
			if(t>2)
				t=0;
				
	}
      }
    }

  } /* namespace freqHop */
} /* namespace gr */

