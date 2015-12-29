///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP. //
//                                                                               //
// Copyright (C) 2015 Edouard Griffiths, F4EXB                                   //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include "IntHalfbandFilter.h"

#if HB_FILTERORDER == 64
	const int32_t IntHalfbandFilter::COEFF[16] = {
		-0.001114417441601693505720538368564120901 * (1 << HB_SHIFT),
		 0.001268007827185253051302527005361753254 * (1 << HB_SHIFT),
		-0.001959831378850490895410230152151598304 * (1 << HB_SHIFT),
		 0.002878308307661380308073439948657323839 * (1 << HB_SHIFT),
		-0.004071361818258721100571850826099762344 * (1 << HB_SHIFT),
		 0.005597288494657440618973431867289036745 * (1 << HB_SHIFT),
		-0.007532345003308904551886371336877346039 * (1 << HB_SHIFT),
		 0.009980346844667375288961963519795972388 * (1 << HB_SHIFT),
		-0.013092614174300500062830820979797863401 * (1 << HB_SHIFT),
		 0.01710934914871829748417297878404497169  * (1 << HB_SHIFT),
		-0.022443558692997273018576720460259821266 * (1 << HB_SHIFT),
		 0.029875811511593811098386197500076377764 * (1 << HB_SHIFT),
		-0.041086352085710403647667021687084343284 * (1 << HB_SHIFT),
		 0.060465467462665789533104998554335907102 * (1 << HB_SHIFT),
		-0.104159517495977321788203084906854201108 * (1 << HB_SHIFT),
		 0.317657589850154464805598308885237202048 * (1 << HB_SHIFT),
	};
#elif HB_FILTERORDER == 48
	const int32_t IntHalfbandFilter::COEFF[12] = {
	   -0.004102576237611492253332112767338912818 * (1 << HB_SHIFT),
		0.003950551047979387886410762575906119309 * (1 << HB_SHIFT),
	   -0.005807875789391703583164350277456833282 * (1 << HB_SHIFT),
		0.00823497890520805998770814682075069868  * (1 << HB_SHIFT),
	   -0.011372226513199541059195851744334504474 * (1 << HB_SHIFT),
		0.015471557140973646315984524335362948477 * (1 << HB_SHIFT),
	   -0.020944996398689276484450516591095947661 * (1 << HB_SHIFT),
		0.028568078132034283034279553703527199104 * (1 << HB_SHIFT),
	   -0.040015143905614086738964374490024056286 * (1 << HB_SHIFT),
		0.059669519431831075095828964549582451582 * (1 << HB_SHIFT),
	   -0.103669138691865420076609893840213771909 * (1 << HB_SHIFT),
		0.317491986549921390015072120149852707982 * (1 << HB_SHIFT)
	};
#elif HB_FILTERORDER == 32
	const int32_t IntHalfbandFilter::mod33[38] = { 31,32,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
						20,21,22,23,24,25,26,27,28,29,30,31,32,0,1,2} ;
	const int32_t IntHalfbandFilter::COEFF[8] = {
		(int32_t)(-0.015956912844043127236437484839370881673 * (1 << HB_SHIFT)),
		(int32_t)( 0.013023031678944928940522274274371739011 * (1 << HB_SHIFT)),
		(int32_t)(-0.01866942273717486777684371190844103694  * (1 << HB_SHIFT)),
		(int32_t)( 0.026550887571157304190005987720724078827 * (1 << HB_SHIFT)),
		(int32_t)(-0.038350314277854319344740474662103224546 * (1 << HB_SHIFT)),
		(int32_t)( 0.058429248652825838128421764849917963147 * (1 << HB_SHIFT)),
		(int32_t)(-0.102889802028955756885153505209018476307 * (1 << HB_SHIFT)),
		(int32_t)( 0.317237706405931241260276465254719369113 * (1 << HB_SHIFT))
	};
#else
#error unsupported filter order
#endif

IntHalfbandFilter::IntHalfbandFilter() : m_ptr(0), m_state(0)
{}
