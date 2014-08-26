/* 
 * File:   AlprNative.h
 * Author: sujay
 *
 * Created on 15 August, 2014, 6:39 PM
 */

#ifndef ALPR_H
#define	ALPR_H
#define CONFIG_FILE	"/etc/openalpr.conf"

#include <jni.h>

#ifdef	__cplusplus
extern "C" {
#endif

jstring _recognize(JNIEnv *, jobject,
		jstring , jstring , jstring ,
		jstring , jint );

std::string detectandshow( Alpr* , cv::Mat , std::string );

std::string errorJsonString(std::string);


#ifdef	__cplusplus
}
#endif

#endif	/* ALPR_H */

