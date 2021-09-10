#include <list>
#include <vector>
#include <string.h>
#include <pthread.h>
#include <cstring>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.h"
#include "Includes/MonoString.h"
#include "Includes/Strings.h"
#include "Includes/Vector3.h"
#include "KittyMemory/MemoryPatch.h"
#include "menu.h"

#include <Substrate/SubstrateHook.h>
#include <Substrate/CydiaSubstrate.h>

#define libName OBFUSCATE("libil2cpp.so")

struct My_Patches {
    MemoryPatch UnlockHats, UnlockPets, UnlockSkins, NoAds;
} hexPatches;

bool NoKillCoolDown, UnlockHats, UnlockPets, UnlockSkins, NoAds;
int Speed = 0, LightRadius = 0;

float (*old_TrueSpeed)(void *instance);
float get_TrueSpeed(void *instance) {
    if (instance != NULL && Speed > 0) {
        return (float) Speed;
    }
    return old_TrueSpeed(instance);
}

float (*old_TrueGhostSpeed)(void *instance);
float get_TrueGhostSpeed(void *instance) {
    if (instance != NULL && Speed > 0) {
        return (float) Speed;
    }
    return old_TrueGhostSpeed(instance);
}

float (*old_CalculateLightRadius)(void *instance);
float get_CalculateLightRadius(void *instance) {
    if (instance != NULL && LightRadius > 0) {
        return (float) LightRadius;
    }
    return old_CalculateLightRadius(instance);
}

void (*old_Update_KillCoolDown)(void *instance);
void Update_KillCoolDown(void *instance) {
    if (NoKillCoolDown) {   
        *(float *) ((uint64_t) instance + 0x44) = 0; 
    }
    return old_Update_KillCoolDown(instance);
}

//Hooking
void *hack_thread(void *) {

    //Check if target lib is loaded
    do {
        sleep(1);
    } while (!isLibraryLoaded(libName));

    //Anti-lib rename
    do {
        sleep(1);
    } while (!isLibraryLoaded("libLaunch.so"));

    hexPatches.UnlockSkins = MemoryPatch::createWithHex(libName, 
                                                    string2Offset(OBFUSCATE("0xE7F534")),
                                                    OBFUSCATE("01 00 A0 E3 1E FF 2F E1"));
    
    hexPatches.UnlockPets = MemoryPatch::createWithHex(libName, 
                                                    string2Offset(OBFUSCATE("0xE7F3AC")),
                                                    OBFUSCATE("01 00 A0 E3 1E FF 2F E1"));
                                                    
   hexPatches.UnlockHats = MemoryPatch::createWithHex(libName,
                                                    string2Offset(OBFUSCATE("0xE7F444")),
                                                    OBFUSCATE("01 00 A0 E3 1E FF 2F E1"));          
                                                    
   hexPatches.NoAds = MemoryPatch::createWithHex(libName,
                                                    string2Offset(OBFUSCATE("0x81E4A4")),
                                                    OBFUSCATE("01 00 A0 E3 1E FF 2F E1"));                                                  
                                                    
    MSHookFunction((void *) getAbsoluteAddress(libName, 0xA65C58), 
    (void *) &get_TrueGhostSpeed,
    (void **) &old_TrueGhostSpeed); 
    
    MSHookFunction((void *) getAbsoluteAddress(libName, 0xA65BBC), 
    (void *) &get_TrueSpeed,
    (void **) &old_TrueSpeed);                
    
    MSHookFunction((void *) getAbsoluteAddress(libName, 0x832ECC), 
    (void *) &get_CalculateLightRadius,
    (void **) &old_CalculateLightRadius);
    
    MSHookFunction((void *) getAbsoluteAddress(libName, 0x8A7D34), 
    (void *) &Update_KillCoolDown,
    (void **) &old_Update_KillCoolDown);         
    
    return NULL;
}

extern "C" {

JNIEXPORT jobjectArray
JNICALL
Java_uk_lgl_modmenu_FloatingModMenuService_getFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    MakeToast(env, context, OBFUSCATE("Modded by Darkside"), Toast::LENGTH_LONG);
            //OBFUSCATE("Category_The Category"), 
            //OBFUSCATE("Toggle_The toggle"),
            //OBFUSCATE("SeekBar_The slider_0_100"),
            //OBFUSCATE("SeekBar_Kittymemory slider example_1_5"),
            //OBFUSCATE("Spinner_The spinner_Items 1,Items 2,Items 3"),
            //OBFUSCATE("Button_The button"),
            //OBFUSCATE("ButtonOnOff_The On/Off button"),
            //OBFUSCATE("CheckBox_The Check Box"),
            //OBFUSCATE("InputValue_Input number"),
            //OBFUSCATE("InputText_Input text"),
            //OBFUSCATE("RadioButton_Radio buttons_OFF,Mod 1,Mod 2,Mod 3"),
    
    
    const char *features[] = {
            OBFUSCATE("Category_Player"), //Not counted         
            OBFUSCATE("SeekBar_Speed_0_30"),//0
            OBFUSCATE("SeekBar_Light Radius_0_10"),//1         
            OBFUSCATE("Category_Impostor"),//Not counted
            OBFUSCATE("Toggle_No Kill CoolDown"),//2       
            OBFUSCATE("Category_Account"), //Not counted
            OBFUSCATE("Toggle_Unlocked Skins"),//3
            OBFUSCATE("Toggle_Unlocked Pets"),//4
            OBFUSCATE("Toggle_Unlocked Hats"),//5
            OBFUSCATE("Category_Misc"), //Not counted
            OBFUSCATE("Toggle_No Ads"),//6
    };

    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
            env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    pthread_t ptid;
    pthread_create(&ptid, NULL, antiLeech, NULL);

    return (ret);
}

JNIEXPORT void JNICALL
Java_uk_lgl_modmenu_Preferences_Changes(JNIEnv *env, jclass clazz, jobject obj,
                                        jint featNum, jstring featName, jint value,
                                        jboolean boolean, jstring str) {

    switch (featNum) {
        case 0:      
            if (value >= 1) {
                Speed = value;
            }
            break;
         case 1:      
            if (value >= 1) {
                LightRadius = value;
            }
            break;              
         case 2:              
            NoKillCoolDown = boolean;        
            break;               
          case 3:
            UnlockSkins = boolean;
            if (UnlockSkins) {            
                hexPatches.UnlockSkins.Modify();             
            } else {
                hexPatches.UnlockSkins.Restore();   
            }
            break;
          case 4:
            UnlockPets = boolean;
            if (UnlockPets) {            
                hexPatches.UnlockPets.Modify();             
            } else {
                hexPatches.UnlockPets.Restore();  
            }
            break;
          case 5:
            UnlockHats = boolean;
            if (UnlockHats) {            
                hexPatches.UnlockHats.Modify();             
            } else {
                hexPatches.UnlockHats.Restore();    
            }   
            break;
         case 6:
            NoAds = boolean;
            if (NoAds) {            
                hexPatches.NoAds.Modify();             
            } else {
                hexPatches.NoAds.Restore();    
            }   
            break;   
      }
   }
}

__attribute__((constructor))
void lib_main() {
 
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}
