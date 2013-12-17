
#include <stdlib.h>

#include <stdio.h>
#include <iostream>
using std::cout; using std::cerr; using std::endl;

#include "mbug.hpp"
#include "mbug_2110.hpp"
#include "mbug_2151.hpp"
#include "mbug_2165.hpp"
#include "mbug_2810.hpp"
#include "mbug_2811.hpp"
#include "mbug_2818.hpp"
#include "mbug_2820.hpp"

//------------------------------------------------

int main(void)
{

// ==== Get device list ====

	mbug::device_list list = mbug::get_device_list();
	if (list.size()==0)
	{
		cout << "No devices found.\n";
		return 0;
	}

	cout << "Attached devices:" << endl;
	for (unsigned i=0; i<list.size(); i++)
		cout << list[i] << endl;
	cout << endl;

// ==== Iterate through attached devices ====

	for (unsigned i=0; i<list.size(); i++)
	{
		cout << endl << "Opening device " << list[i] << endl;
		int type = atoi( list[i].substr(5,4).c_str() );

		try
		{
			switch (type)
			{
				// -------------------------------------------------------------
				case 2110:	// MBUG-2110 GPIO 
				{
					mbug::mbug_2110 dev;

					cout << "Set GPIO ";
					for (int ch=0; ch<12; ch++)
					{
						cout << '.';
						dev.set_gpio( 1<<ch );
					}
					cout << endl;
					
					cout << "Set PWM ";
					dev.enable_pwm(1);
					for (int d=0; d<1024; d+=50)
					{
						cout << '.';
						dev.set_pwm(d);
					}
					dev.enable_pwm(0);
					cout << endl;
					
					cout << "Get GPIO: ";
					int gpio = dev.get_gpio();
					for (int bit=0; bit<12; bit++ )
						cout << (gpio&(1<<bit) ? '1' : '0');
					cout << endl;

					cout << "Read ADC: ";
					mbug::mbug_2110::adc_list adc;
					adc = dev.read_adc();
					for (int ch=0; ch<8; ch++ )
						cout << adc[ch] << (ch<8 ? ", " : "");
					cout << endl;
				}

				//-------------------------------------------------------------
				case 2165:	// MBUG-2165 IR Transmitter
				{
					mbug::mbug_2165 dev;

					dev.reset();
					dev.set_mod_freq( 38e3 );	// Carrier frequency: 38 kHz
					dev.set_timebase( 10e-6 );	// Interval unit: 10 us
					dev.set_iterations( 3 );
					unsigned char seq_a[] = {1,2,3,4,5,6,7,8};
					dev.set_sequence( seq_a, sizeof(seq_a), TX_MODE_TIMED_8 );
					dev.start();
					while (dev.get_busy()) {
						; // wait
					}

					dev.set_iterations(0);  // infinitely
					unsigned char seq_b[] = {0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF};
					dev.set_sequence( seq_b, sizeof(seq_b), TX_MODE_BITSTREAM ); 
					dev.start();
					dev.stop_gracefully();
					while (dev.get_busy()) {
						; // wait
					}

					dev.set_iterations(5);
					dev.set_clock_div(1);
					std::vector<unsigned short> seq_c(8);
					for (unsigned int i=0, v=250; i<seq_c.size(); i++, v*=2)  seq_c[i] = v;
					dev.set_sequence( seq_c );
					dev.start();
				}

				//-------------------------------------------------------------
				case 2151:	// MBUG-2151 433MHz Transmitter
				{
					mbug::mbug_2151 dev;

					dev.reset();
					dev.set_interval( 10e-6 );	// Interval unit: 10 us
					dev.set_iterations( 3 );
					unsigned char seq_a[] = {1,2,3,4,5,6,7,8};
					dev.set_sequence( seq_a, sizeof(seq_a), TX_MODE_TIMED_8 );
					dev.start();
					while (dev.get_busy()) {
						; // wait
					}

					dev.set_iterations(0);  // infinitely
					unsigned char seq_b[] = {0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF};
					dev.set_sequence( seq_b, sizeof(seq_b), TX_MODE_BITSTREAM ); 
					dev.start();
					dev.stop_gracefully();
					while (dev.get_busy()) {
						; // wait
					}

					dev.set_iterations(5);
					dev.set_clock_div(1);
					std::vector<unsigned short> seq_c(8);
					for (unsigned int i=0, v=250; i<seq_c.size(); i++, v*=2)  seq_c[i] = v;
					dev.set_sequence( seq_c );
					dev.start();
				}
				
				//-------------------------------------------------------------
				case 2810:	// MBUG-2810 Ambient Thermometer
				{
					mbug::mbug_2810 dev;
					double tem = dev.read();
					printf("Temperature: %.2f\n", tem);
					break;
				}

				//-------------------------------------------------------------
				case 2811:	// MBUG-2811 Thermometer
				{
					mbug::mbug_2811 dev;
					double tem = dev.read();
					printf("Temperature: %.2f\n", tem);
					break;
				}

				// -------------------------------------------------------------
				case 2818:	// MBUG-2818 8-channel Thermometer 
				{
					mbug::mbug_2818 dev;
					std::vector<double> tem = dev.read_all();
					cout << "Temperatures:" << endl;
					for (unsigned i=0; i<tem.size(); i++)
					{
						if (tem[i]==NOT_A_TEMPERATURE)
							 printf("ch%d: -----\n", i);
						else printf("ch%d: %.2f\n", i, tem[i]);
					}
					break;
				}

				// -------------------------------------------------------------
				case 2820:	// MBUG-2820 environemter
				{
					mbug::mbug_2820 dev;
					std::vector<double> meas = dev.read();
					double temperature = meas[0];
					double humidity = meas[1];
					cout << "Temperature: " << temperature << endl;
					cout << "Humidity: " << humidity << endl;
					break;
				}

				// -------------------------------------------------------------
				default:
				{
					cout << "No action defined for this device." << endl;
					break;
				}
			}
		}
		catch (mbug::error& err)
		{
			cerr << err.what();
		}

	} // End for devices in list

	return 0;
}


//------------------------------------------------


