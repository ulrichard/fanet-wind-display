/*
 * clamp.h
 *
 *  Created on: 29.06.2018
 *      Author: skytraxx
 */

#ifndef CLAMP_H_
#define CLAMP_H_


	#define clampf(val, minv, maxv)		val = fmaxf((minv), fminf((maxv), (val)))
	#define clamp(val, minv, maxv)		val = max((minv), min((maxv), (val)))



#endif /* CLAMP_H_ */
