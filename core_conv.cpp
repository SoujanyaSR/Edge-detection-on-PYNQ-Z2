#include "core_conv.hpp"
#include "hls_math.h"
#include "math.h"

void conv1(hls::stream<AXI_STREAMU> & inStream, hls::stream<AXI_STREAMU> & outStream, int  rows, int  cols)
{
#pragma HLS DATAFLOW
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL
#pragma HLS INTERFACE s_axilite port=rows bundle=CRTL
#pragma HLS INTERFACE s_axilite port=cols bundle=CRTL
#pragma HLS INTERFACE axis register both port=outStream
#pragma HLS INTERFACE axis register both port=inStream

	//Defining LineBuffers
	hls::LineBuffer<KERNEL_SIZE1, MAX_WIDTH, unsigned char> linebuff; //positive pixel values
	hls::Window<KERNEL_SIZE1,KERNEL_SIZE1, short> window; //integer weights


	AXI_STREAMU valIn;
	AXI_STREAMU valOut;

	for(int i =0; i<2; i++)
    for(int j=0; j<cols; j++)

		{
			#pragma HLS PIPELINE
			valIn = inStream.read();
			linebuff.shift_pixels_up(j);
			linebuff.insert_bottom_row(valIn.data, j);
		}

	for(int i=2; i<rows; i++)
	{
		for(int j=0; j<cols; j++)
		{
			#pragma HLS PIPELINE
			//put the pixel in the bottom row
			valIn = inStream.read();
			linebuff.shift_pixels_up(j);
		    linebuff.insert_bottom_row(valIn.data, j);

		    //Get the receptive field

		    //read the values to window to the last column
		    window.shift_pixels_left();
		    window.insert_pixel(linebuff.getval(0,j),0,2);
		    window.insert_pixel(linebuff.getval(1,j),1,2);
		    window.insert_pixel(linebuff.getval(2,j),2,2);

		    if(j>=2)
		    {
		    	int Hsum=0;
		    	int Vsum=0;
		    	int sum;

			    for(int m =0; m<KERNEL_SIZE1; m++)
				{
					for(int n=0; n<KERNEL_SIZE1; n++)
					{
						#pragma HLS PIPELINE
						Hsum += window.getval(m,n)* Gx[m][n];
						Vsum += window.getval(m,n)* Gy[m][n];
					}

				}

			    Hsum = pow(Hsum,2);
			    Vsum = pow(Vsum,2);

			    sum =hls::sqrt(Hsum+Vsum);

			    if(sum > 75)
			    	sum = 255;
			    else
			    	sum = 0;

			    valOut.data = sum;
			    //valOut.last = 1;
			    valOut.dest = valIn.dest;
			    valOut.id = valIn.id;
			    valOut.keep = valIn.keep;
			    if(i==rows-1 && j == cols-1)
			    	valOut.last = 1;
			    else
			    	valOut.last = 0;
			    valOut.strb = valIn.strb;
			    valOut.user = valIn.user;
			    outStream.write(valOut);


		    }

		}

	}

}
