#include <string>
#include <sstream>
#include <cstdio>
#include <iostream>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

// open alpr includes
#include "support/filesystem.h"
#include "support/timing.h"
#include "alpr.h"
#include "cjson.h"

#include "AlprJNIWrapper.h"
#include "AlprNative.h"

JNIEXPORT jstring JNICALL Java_org_openalpr_AlprJNIWrapper_recognize(JNIEnv *env,
		jobject object, jstring jimgFilePath, jint jtopN)
{
	jstring defaultCountry = env->NewStringUTF("us");
	jstring defaultRegion = env->NewStringUTF("");
	jstring defaultConfigFilePath = env->NewStringUTF(CONFIG_FILE);
	return _recognize(env, object, defaultCountry, defaultRegion, jimgFilePath, defaultConfigFilePath, jtopN);
}

JNIEXPORT jstring JNICALL Java_org_openalpr_AlprJNIWrapper_recognizeWithCountryNRegion(
		JNIEnv *env, jobject object, jstring jcountry,
		jstring jregion, jstring jimgFilePath, jint jtopN)
{
	jstring defaultConfigFilePath = env->NewStringUTF(CONFIG_FILE);
	return _recognize(env, object, jcountry, jregion, jimgFilePath, defaultConfigFilePath, jtopN);
}

JNIEXPORT jstring JNICALL Java_org_openalpr_AlprJNIWrapper_recognizeWithCountryRegionNConfig
  (JNIEnv *env, jobject object, jstring jcountry, jstring jregion,
		  jstring jimgFilePath, jstring jconfigFilePath, jint jtopN)
{
	return _recognize(env, object, jcountry, jregion, jimgFilePath, jconfigFilePath, jtopN);
}

jstring _recognize(JNIEnv *env, jobject object,
		jstring jcountry, jstring jregion, jstring jimgFilePath,
		jstring jconfigFilePath, jint jtopN)
{

	const char* countryChars = env->GetStringUTFChars(jcountry, NULL);

	std::string country(countryChars);

	env->ReleaseStringUTFChars(jcountry, countryChars);

	if(country.empty())
	{
		country = "us";
	}

	const char* configFilePathChars = env->GetStringUTFChars(jconfigFilePath, NULL);

	std::string configFilePath(configFilePathChars);

	env->ReleaseStringUTFChars(jconfigFilePath, configFilePathChars);

	if(configFilePath.empty())
	{
		configFilePath = "/etc/openalpr/openalpr.conf";
	}

	const char* imgFilePath = env->GetStringUTFChars(jimgFilePath, NULL);

	int topN = jtopN;

	std::string response = "";

	cv::Mat frame;
	Alpr alpr(country, configFilePath);

	const char* regionChars = env->GetStringUTFChars(jregion, NULL);

	std::string region(regionChars);

	env->ReleaseStringUTFChars(jregion, regionChars);

	if(region.empty())
	{
		alpr.setDetectRegion(true);
		alpr.setDefaultRegion(region);
	}


	alpr.setTopN(topN);

	if (alpr.isLoaded() == false) {
		env->ReleaseStringUTFChars(jimgFilePath, imgFilePath);
		response = errorJsonString("Error initializing Open Alpr");
		return env->NewStringUTF(response.c_str());
	}

	if(fileExists(imgFilePath))
	{
		frame = cv::imread(imgFilePath);
		response = detectandshow(&alpr, frame, "");
	}
	else
	{
		response = errorJsonString("Image file not found");
	}
	env->ReleaseStringUTFChars(jimgFilePath, imgFilePath);
	return env->NewStringUTF(response.c_str());
}

JNIEXPORT jstring JNICALL Java_org_openalpr_AlprJNIWrapper_version
  (JNIEnv *env, jobject object)
{
	return env->NewStringUTF(Alpr::getVersion().c_str());
}

std::string detectandshow(Alpr* alpr, cv::Mat frame, std::string region) 
{
	std::vector < uchar > buffer;
	std::string resultJson = "";
	cv::imencode(".bmp", frame, buffer);

	timespec startTime;
	getTime(&startTime);

	std::vector < AlprResult > results = alpr->recognize(buffer);

	timespec endTime;
	getTime(&endTime);
	double totalProcessingTime = diffclock(startTime, endTime);

	if (results.size() > 0)
	{
		resultJson = alpr->toJson(results, totalProcessingTime);
	}

	return resultJson;
}

std::string errorJsonString(std::string msg) 
{
	cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddTrueToObject(root, "error");
	cJSON_AddStringToObject(root, "msg", msg.c_str());

	char *out;
	out = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	std::string response(out);

	free(out);
	return response;
}
