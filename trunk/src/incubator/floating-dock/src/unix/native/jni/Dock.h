/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_jdesktop_jdic_dock_internal_impl_GnomeDockService */

#ifndef _Included_org_jdesktop_jdic_dock_internal_impl_GnomeDockService
#define _Included_org_jdesktop_jdic_dock_internal_impl_GnomeDockService
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_jdesktop_jdic_dock_internal_impl_GnomeDockService
 * Method:    createDockWindow
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_org_jdesktop_jdic_dock_internal_impl_GnomeDockService_createDockWindow
  (JNIEnv *, jobject);

/*
 * Class:     org_jdesktop_jdic_dock_internal_impl_GnomeDockService
 * Method:    getWidget
 * Signature: (JIIII)J
 */
JNIEXPORT jlong JNICALL Java_org_jdesktop_jdic_dock_internal_impl_GnomeDockService_getWidget
  (JNIEnv *, jobject, jlong, jint, jint, jint, jint);

/*
 * Class:     org_jdesktop_jdic_dock_internal_impl_GnomeDockService
 * Method:    adjustSizeHints
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_org_jdesktop_jdic_dock_internal_impl_GnomeDockService_adjustSizeHints
  (JNIEnv *, jobject, jlong, jint, jint);

/*
 * Class:     org_jdesktop_jdic_dock_internal_impl_GnomeDockService
 * Method:    locateDock
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_jdesktop_jdic_dock_internal_impl_GnomeDockService_locateDock
  (JNIEnv *, jclass);

/*
 * Class:     org_jdesktop_jdic_dock_internal_impl_GnomeDockService
 * Method:    eventLoop
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_jdesktop_jdic_dock_internal_impl_GnomeDockService_eventLoop
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif