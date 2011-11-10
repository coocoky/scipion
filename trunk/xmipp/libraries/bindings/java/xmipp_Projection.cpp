#include <jni.h>
#include "xmipp_Projection.h"
#include "xmipp_ImageDouble.h"
#include "xmipp_ExceptionsHandler.h"
#include <data/xmipp_image.h>
#include <data/projection.h>
#include "xmipp_InternalData.h"

JNIEXPORT void JNICALL Java_xmipp_Projection_storeIds
(JNIEnv *env, jclass cls) {
	Projection_peerId = env->GetFieldID(cls, "peer", "J");
}

JNIEXPORT void JNICALL Java_xmipp_Projection_create
(JNIEnv *env, jobject jobj) {
	Projection *projection = new Projection();
	env->SetLongField(jobj, Projection_peerId, (long)projection);
}

JNIEXPORT void JNICALL Java_xmipp_Projection_destroy
(JNIEnv *env, jobject jobj) {
	Projection *projection = GET_INTERNAL_PROJECTION(jobj);
	delete projection;
	projection = NULL;
	env->SetLongField(jobj, Projection_peerId, (long)projection);
}

JNIEXPORT void JNICALL Java_xmipp_Projection_reset
(JNIEnv *env, jobject jobj, jint h, jint w) {
	std::string msg = "";
	Projection *projection = GET_INTERNAL_PROJECTION(jobj);

	if(projection != NULL) {
		try {
			projection->reset((int) h, (int) w);
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Projection is NULL";
	}

	// If there was an exception, sends it to java environment.
	if(!msg.empty()) {
		handleXmippException(env, msg);
	}
}

JNIEXPORT void JNICALL Java_xmipp_Projection_projectVolume
(JNIEnv *env, jclass cls, jobject jvolume, jobject jprojection, jdouble tilt, jdouble rot, jdouble pshi) {
	std::string msg = "";
	Image<double> *volume = GET_INTERNAL_IMAGE(jvolume);
	Projection *projection = GET_INTERNAL_PROJECTION(jprojection);

	if(volume != NULL) {
		if(projection != NULL) {
			try {
				int w = XSIZE(projection->data);
				int h = YSIZE(projection->data);
				projectVolume((*volume)(), *projection, h, w, (double) rot, (double) tilt, (double) pshi);
			} catch (XmippError xe) {
				msg = xe.getDefaultMessage();
			} catch (std::exception& e) {
				msg = e.what();
			} catch (...) {
				msg = "Unhandled exception";
			}
		} else {
			msg = "projection is NULL";
		}
	} else {
		msg = "volume is NULL";
	}

	// If there was an exception, sends it to java environment.
	if(!msg.empty()) {
		handleXmippException(env, msg);
	}
}

JNIEXPORT void JNICALL Java_xmipp_Projection_write
(JNIEnv *env, jobject jobj, jstring filename) {
	std::string msg = "";
	Projection *projection = GET_INTERNAL_PROJECTION(jobj);

	if (projection != NULL) {
		try {
			const char * fnStr = env->GetStringUTFChars(filename, false);

			projection->write(fnStr);
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Projection is null";
	}

	// If there was an exception, sends it to java environment.
	if(!msg.empty()) {
		handleXmippException(env, msg);
	}
}

JNIEXPORT jdoubleArray JNICALL Java_xmipp_Projection_getData(JNIEnv *env,
		jobject jobj) {
	std::string msg = "";
	Projection *projection = GET_INTERNAL_PROJECTION(jobj);

	if (projection != NULL) {
		try {
			size_t size = projection->getSize();
			jdoubleArray array = env->NewDoubleArray(size);
			env->SetDoubleArrayRegion(array, 0, size, MULTIDIM_ARRAY(
					projection->data));

			return array;
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Projection is null";
	}

	// If there was an exception, sends it to java environment.
	if (!msg.empty()) {
		handleXmippException(env, msg);
	}

	return (jdoubleArray) NULL;
}

JNIEXPORT jdoubleArray JNICALL Java_xmipp_Projection_getData__JI(JNIEnv *env,
		jobject jobj, jlong nimage, jint nslice) {
	std::string msg = "";
	Projection *projection = GET_INTERNAL_PROJECTION(jobj);

	if (projection != NULL) {
		try {
			int w = XSIZE(projection->data);
			int h = YSIZE(projection->data);
			size_t size = w * h;

			MultidimArray<double> mdarray(size);
			projection->data.getSlice(nslice, mdarray, nimage);

			jdoubleArray array = env->NewDoubleArray(size);
			env->SetDoubleArrayRegion(array, 0, size, MULTIDIM_ARRAY(mdarray));

			return array;
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Projection is null";
	}

	// If there was an exception, sends it to java environment.
	if (!msg.empty()) {
		handleXmippException(env, msg);
	}

	return (jdoubleArray) NULL;
}

JNIEXPORT jint JNICALL Java_xmipp_Projection_getXsize(JNIEnv *env, jobject jobj) {
	std::string msg = "";
	Projection *projection = GET_INTERNAL_PROJECTION(jobj);

	if (projection != NULL) {
		try {
			return XSIZE(projection->data);
		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Projection is null";
	}

	// If there was an exception, sends it to java environment.
	if (!msg.empty()) {
		handleXmippException(env, msg);
	}

	return 0;
}

JNIEXPORT jint JNICALL Java_xmipp_Projection_getYsize(JNIEnv *env, jobject jobj) {
	std::string msg = "";
	Projection *projection = GET_INTERNAL_PROJECTION(jobj);

	if (projection != NULL) {
		try {
			return YSIZE(projection->data);

		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Projection is null";
	}

	// If there was an exception, sends it to java environment.
	if (!msg.empty()) {
		handleXmippException(env, msg);
	}

	return 0;
}

JNIEXPORT jint JNICALL Java_xmipp_Projection_getZsize(JNIEnv *env, jobject jobj) {
	std::string msg = "";
	Projection *projection = GET_INTERNAL_PROJECTION(jobj);

	if (projection != NULL) {
		try {
			return ZSIZE(projection->data);

		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Projection is null";
	}

	// If there was an exception, sends it to java environment.
	if (!msg.empty()) {
		handleXmippException(env, msg);
	}

	return 0;
}

JNIEXPORT void JNICALL Java_xmipp_Projection_printShape
(JNIEnv *env, jobject jobj) {
	std::string msg = "";
	Projection *projection = GET_INTERNAL_PROJECTION(jobj);

	if (projection != NULL) {
		try {
			std::cout << (*projection) << std::endl;

		} catch (XmippError xe) {
			msg = xe.getDefaultMessage();
		} catch (std::exception& e) {
			msg = e.what();
		} catch (...) {
			msg = "Unhandled exception";
		}
	} else {
		msg = "Projection is null";
	}

	// If there was an exception, sends it to java environment.
	if(!msg.empty()) {
		handleXmippException(env, msg);
	}
}
