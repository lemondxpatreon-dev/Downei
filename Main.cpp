#include <list>
#include <atomic>
#include <cstdint>
#include <vector>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <unordered_map>
#include <cstring>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include <cctype>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include "Includes/obfuscate.h"
#include "Includes/Logger.h"
#include "Includes/Utils.h"
#include "KittyMemory/MemoryPatch.h"
#include "Includes/Strings.h"
#include "Includes/Macros.h"
#include "Includes/Array.h"
#include "Includes/StringUtils.cpp"

#define targetLibName OBFUSCATE("libil2cpp.so")

static bool inTourx;
static std::atomic<bool>  utilitiesReady{false};
static int setCustomGamemode = 0;
static std::atomic<bool>  settingsPatched{false};
static std::atomic<int>   tauntOption{1};

struct Vector2 {
    float x;
    float y;
};

struct Rect {
    float x;
    float y;
    float width;
    float height;
};

void* (*createSprite)(void* texture, Rect rect, Vector2 pivot) = nullptr;
void* (*gameObjectFind)(String* name) = nullptr;
void* (*objectGetComponent)(void* instance, String* typeName) = nullptr;
void* (*getTransform)(void* instance) = nullptr;
void* (*getParent)(void* instance) = nullptr;
void* (*objectInstantiate)(void* original, void* parent) = nullptr;
void* (*objectDestroy)(void* obj) = nullptr;
void* (*getTypeFromString)(String* typeName, bool throwOnError)                      = nullptr;
void* (*transformSetPosition)(void* transform, void* value) = nullptr;
void* (*transformGetPosition)(void* instance) = nullptr;
void* (*vector3Ctor)(float x, float y, float z) = nullptr;
void* (*gameObjectSetActive)(void* _this, bool value) = nullptr;
void* (*objectSetName)(void* instance, String* value) = nullptr;
void* (*componentSetActive)(void* instance, bool value) = nullptr;
void* (*loadSprite)(String* path, void* systemTypeInstance) = nullptr;
String* (*objectGetName)(void* instance) = nullptr;
void* (*toVec)(void*, float z, float y, float x) = nullptr;
void* (*objectGetComponentInChildren)(void* instance, void* type) = nullptr;
void* (*transformTranslate)(void* instance, float x, float y, float z) = nullptr;
void* (*componentGetTransform)(void* instance) = nullptr;
void* (*componentGetGameObject)(void* instance) = nullptr;
Vector2 (*rectTransformGetAnchorMin)(void* instance) = nullptr;
void (*rectTransformSetAnchorMin)(void* instance, Vector2 value) = nullptr;
Vector2 (*rectTransformGetAnchorMax)(void* instance) = nullptr;
void (*rectTransformSetAnchorMax)(void* instance, Vector2 value) = nullptr;
Vector2 (*rectTransformGetAnchoredPosition)(void* instance) = nullptr;
void (*rectTransformSetAnchoredPosition)(void* instance, Vector2 value) = nullptr;
void (*rectTransformSetSizeDelta)(void* instance, Vector2 value) = nullptr;
String* (*userGetUsername)(void* instance) = nullptr;
void* (*userGetMe)() = nullptr;
void (*tmpSetText)(void* instance, String* value) = nullptr;
void* (*findObject)(void* type) = nullptr;
void* (*activatorCreateInstance)(void* type) = nullptr;
String* (*getTraducao)(String* key) = nullptr;
void* (*scriptableObjectCreateInstance)(void* type) = nullptr;
void* (*createUri)(String* s) = nullptr;
void* (*cloneCustomRoomProperties)(void* instance) = nullptr;
void* (*getHashtableItem)(void* hashtable, String* key) = nullptr;
void* levelManagerInstance;
void* (*openPopup)(void*, String*, float, bool) = nullptr;
void* (*setPopupItems)(void*, String*, String*) = nullptr;
void* (*getCurrencySprite)(void* instance, String* spriteID) = nullptr;
bool (*imageConversionLoadImage)(void* texture, void* data, bool markNonReadable) = nullptr;
void* (*loadTextureFromFile)(String* path, bool ignoreMipMapLimit) = nullptr;
int (*textureGetWidth)(void* texture) = nullptr;
int (*textureGetHeight)(void* texture) = nullptr;

// UnityEngine.UI.Image setters. Using the real property setter is important:
// writing m_Sprite directly does not reliably dirty/rebuild the Canvas.
void (*imageSetSprite)(void* image, void* sprite) = nullptr;
void (*imageSetPreserveAspect)(void* image, bool value) = nullptr;
void (*imageSetType)(void* image, int value) = nullptr;

void* (*transformFind)(void* transform, String* name) = nullptr;
void* (*transformSetParent)(void* transform, void* parent, bool worldPositionStays) = nullptr;
void (*_setTMPText)(void* textComponent, const char* text) = nullptr;
void (*_setFirstTMPText)(void* gameObject, const char* text) = nullptr;
void* (*cachedBgSprite)(void) = nullptr;
void (*_preloadBg)(void) = nullptr;
void (*_applyBgSprite)(void* instance, void* sprite) = nullptr;
extern void* (*oldMapConfigGetter)(void* instance);


static String* respawnLegendaryString = nullptr;
static String* respawnHourString = nullptr;
static String* respawnDashString = nullptr;
void* (*actacteCreateInstanceParams)(void* type, void* args[]) = nullptr;


static void* tmpTextMeshPro = nullptr;
static bool isAnimating = false;
static float hueOffset = 0.0f;

void* (*levelEntry)(void* instance, void* levelDef);
void* customLevelDef;
static bool addedMap = false;

int userId;
    

void* (*getType)(String* typeName);

void* (*oldLogin)(void* instance, void* callback);
void* login(void* instance, void* callback) {
    if (instance) {
        void* user = *(void**)((uintptr_t)instance + 0x20);
        if (user) {
            userId = *(int*)((uint8_t*)user + 0x10);
        }
    }
    return oldLogin(instance, callback);
}

int getUserId() {
    if (userId != 0) return userId;
    std::srand(std::time(nullptr));
    return std::rand() % 100 + 1;
}

void* loadSpriteFunc(char* spriteID) {
    void* initializerType = getType(String::Create("Stumble.Initializer, Assembly-CSharp"));
    if (!initializerType) {
        return nullptr;
    }
    
    void* initializerInstance = findObject(initializerType);
    if (!initializerInstance) {
        return nullptr;
    }
    
    String* spriteIDString = String::Create(spriteID);
    if (!spriteIDString) {
        return nullptr;
    }
   
    void* uiArtInstance = *(void**)((uintptr_t)initializerInstance + 0xB8);
    
    return getCurrencySprite(uiArtInstance, spriteIDString);
}




static char kProfileIdImagePath[] = "src/main/assets/id.png";
static char kLoginBackgroundImagePath[] = "src/main/assets/background.png";


// ============================================================================
// Native C++ remote image loader
// Replaces the C# UnityWebRequest/MelonCoroutines part.
// Downloads are performed through Android's Java URL/InputStream API via JNI,
// while Unity texture/sprite creation stays on the Unity thread.
// ============================================================================

static const char* kRemotePlayBackgroundUrl = "https://i.imgur.com/uyhtQWF.png";
static const char* kRemoteLoginBackgroundUrl = "https://files.catbox.moe/zal3h8.png";

static std::atomic<int> gPlayBackgroundDownloadState{0}; // 0 idle, 1 loading, 2 ready, -1 failed
static void* loadLocalSprite(char* path);
static std::atomic<int> gLoginBackgroundDownloadState{0};
static void* gPlayBackgroundSprite = nullptr;
static void* gLoginBackgroundSprite = nullptr;
static void* gCurrentPlayViewInstance = nullptr;
static void* gCurrentLoginViewInstance = nullptr;

static std::string getProcessPackageName()
{
    std::ifstream f("/proc/self/cmdline", std::ios::in | std::ios::binary);
    std::string name;
    if (f.good())
        std::getline(f, name, '\0');

    if (name.empty())
        name = "com.stumbleguys.game";

    for (char& c : name)
    {
        if (c == '\0' || c == '/' || c == ' ')
            c = '_';
    }
    return name;
}

static bool downloadFileWithAndroidJNI(const char* url, const char* outputPath)
{
    if (!url || !outputPath)
        return false;

    // Use Android's HttpURLConnection instead of URL.openStream().
    // Imgur may redirect, and openStream() gives us no useful HTTP status.
    using GetCreatedJavaVMs_t = jint (*)(JavaVM**, jsize, jsize*);

    JavaVM* vm = nullptr;
    jsize vmCount = 0;

    GetCreatedJavaVMs_t getCreatedJavaVMs =
        reinterpret_cast<GetCreatedJavaVMs_t>(
            dlsym(RTLD_DEFAULT, "JNI_GetCreatedJavaVMs")
        );

    void* artHandle = nullptr;
    if (!getCreatedJavaVMs)
    {
        artHandle = dlopen("libart.so", RTLD_NOW);
        if (artHandle)
        {
            getCreatedJavaVMs =
                reinterpret_cast<GetCreatedJavaVMs_t>(
                    dlsym(artHandle, "JNI_GetCreatedJavaVMs")
                );
        }
    }

    if (!getCreatedJavaVMs ||
        getCreatedJavaVMs(&vm, 1, &vmCount) != JNI_OK ||
        vm == nullptr || vmCount <= 0)
    {
        if (artHandle)
            dlclose(artHandle);
        return false;
    }

    JNIEnv* env = nullptr;
    bool attached = false;

    jint envResult =
        vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);

    if (envResult == JNI_EDETACHED)
    {
        if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK)
        {
            if (artHandle)
                dlclose(artHandle);
            return false;
        }
        attached = true;
    }
    else if (envResult != JNI_OK || env == nullptr)
    {
        if (artHandle)
            dlclose(artHandle);
        return false;
    }

    bool success = false;

    do
    {
        jclass urlClass = env->FindClass("java/net/URL");
        jclass httpClass = env->FindClass("java/net/HttpURLConnection");
        jclass inputStreamClass = env->FindClass("java/io/InputStream");
        jclass fileOutputStreamClass =
            env->FindClass("java/io/FileOutputStream");

        if (!urlClass || !httpClass || !inputStreamClass ||
            !fileOutputStreamClass)
            break;

        jmethodID urlCtor =
            env->GetMethodID(urlClass, "<init>", "(Ljava/lang/String;)V");
        jmethodID openConnection =
            env->GetMethodID(
                urlClass,
                "openConnection",
                "()Ljava/net/URLConnection;"
            );

        jmethodID setConnectTimeout =
            env->GetMethodID(
                httpClass,
                "setConnectTimeout",
                "(I)V"
            );
        jmethodID setReadTimeout =
            env->GetMethodID(
                httpClass,
                "setReadTimeout",
                "(I)V"
            );
        jmethodID setRequestProperty =
            env->GetMethodID(
                httpClass,
                "setRequestProperty",
                "(Ljava/lang/String;Ljava/lang/String;)V"
            );
        jmethodID setFollowRedirects =
            env->GetMethodID(
                httpClass,
                "setInstanceFollowRedirects",
                "(Z)V"
            );
        jmethodID getResponseCode =
            env->GetMethodID(
                httpClass,
                "getResponseCode",
                "()I"
            );
        jmethodID getInputStream =
            env->GetMethodID(
                httpClass,
                "getInputStream",
                "()Ljava/io/InputStream;"
            );
        jmethodID disconnect =
            env->GetMethodID(
                httpClass,
                "disconnect",
                "()V"
            );

        jmethodID inputRead =
            env->GetMethodID(inputStreamClass, "read", "([BII)I");
        jmethodID inputClose =
            env->GetMethodID(inputStreamClass, "close", "()V");

        jmethodID fileCtor =
            env->GetMethodID(
                fileOutputStreamClass,
                "<init>",
                "(Ljava/lang/String;)V"
            );
        jmethodID fileWrite =
            env->GetMethodID(
                fileOutputStreamClass,
                "write",
                "([BII)V"
            );
        jmethodID fileClose =
            env->GetMethodID(fileOutputStreamClass, "close", "()V");

        if (!urlCtor || !openConnection || !setConnectTimeout ||
            !setReadTimeout || !setRequestProperty || !setFollowRedirects ||
            !getResponseCode ||
            !getInputStream || !disconnect || !inputRead || !inputClose ||
            !fileCtor || !fileWrite || !fileClose)
            break;

        jstring jUrl = env->NewStringUTF(url);
        if (!jUrl)
            break;

        jobject urlObject = env->NewObject(urlClass, urlCtor, jUrl);
        if (!urlObject || env->ExceptionCheck())
        {
            env->ExceptionClear();
            env->DeleteLocalRef(jUrl);
            break;
        }

        jobject connection =
            env->CallObjectMethod(urlObject, openConnection);

        if (!connection || env->ExceptionCheck())
        {
            env->ExceptionClear();
            env->DeleteLocalRef(urlObject);
            env->DeleteLocalRef(jUrl);
            break;
        }

        env->CallVoidMethod(connection, setConnectTimeout, 15000);
        env->CallVoidMethod(connection, setReadTimeout, 20000);

        jstring uaName = env->NewStringUTF("User-Agent");
        jstring uaValue =
            env->NewStringUTF("Mozilla/5.0 (Android) StumbleSharkz");
        jstring acceptName = env->NewStringUTF("Accept");
        jstring acceptValue =
            env->NewStringUTF("image/avif,image/webp,image/apng,image/png,image/*,*/*;q=0.8");

        if (uaName && uaValue)
            env->CallVoidMethod(
                connection,
                setRequestProperty,
                uaName,
                uaValue
            );

        if (acceptName && acceptValue)
            env->CallVoidMethod(
                connection,
                setRequestProperty,
                acceptName,
                acceptValue
            );

        env->CallVoidMethod(connection, setFollowRedirects, JNI_TRUE);

        if (env->ExceptionCheck())
            env->ExceptionClear();

        jint responseCode =
            env->CallIntMethod(connection, getResponseCode);

        if (env->ExceptionCheck())
        {
            env->ExceptionClear();
            responseCode = -1;
        }

        // Accept normal success responses and redirects handled by
        // HttpURLConnection. Anything else is a real download failure.
        if (responseCode < 200 || responseCode >= 300)
        {
            env->CallVoidMethod(connection, disconnect);
            if (uaName) env->DeleteLocalRef(uaName);
            if (uaValue) env->DeleteLocalRef(uaValue);
            if (acceptName) env->DeleteLocalRef(acceptName);
            if (acceptValue) env->DeleteLocalRef(acceptValue);
            env->DeleteLocalRef(connection);
            env->DeleteLocalRef(urlObject);
            env->DeleteLocalRef(jUrl);
            break;
        }

        jobject input =
            env->CallObjectMethod(connection, getInputStream);

        if (!input || env->ExceptionCheck())
        {
            env->ExceptionClear();
            env->CallVoidMethod(connection, disconnect);
            if (uaName) env->DeleteLocalRef(uaName);
            if (uaValue) env->DeleteLocalRef(uaValue);
            env->DeleteLocalRef(connection);
            env->DeleteLocalRef(urlObject);
            env->DeleteLocalRef(jUrl);
            break;
        }

        // Write to a temporary file first. This prevents a half-downloaded
        // image from being mistaken for a valid cached background.
        std::string tempPath =
            std::string(outputPath) + ".download";

        jstring jPath = env->NewStringUTF(tempPath.c_str());
        if (!jPath)
        {
            env->CallVoidMethod(input, inputClose);
            env->CallVoidMethod(connection, disconnect);
            if (uaName) env->DeleteLocalRef(uaName);
            if (uaValue) env->DeleteLocalRef(uaValue);
            env->DeleteLocalRef(input);
            env->DeleteLocalRef(connection);
            env->DeleteLocalRef(urlObject);
            env->DeleteLocalRef(jUrl);
            break;
        }

        // Remove an old partial download.
        unlink(tempPath.c_str());

        jobject output =
            env->NewObject(fileOutputStreamClass, fileCtor, jPath);

        if (!output || env->ExceptionCheck())
        {
            env->ExceptionClear();
            env->CallVoidMethod(input, inputClose);
            env->CallVoidMethod(connection, disconnect);
            unlink(tempPath.c_str());
            if (uaName) env->DeleteLocalRef(uaName);
            if (uaValue) env->DeleteLocalRef(uaValue);
            env->DeleteLocalRef(input);
            env->DeleteLocalRef(connection);
            env->DeleteLocalRef(urlObject);
            env->DeleteLocalRef(jUrl);
            env->DeleteLocalRef(jPath);
            break;
        }

        jbyteArray buffer = env->NewByteArray(16 * 1024);
        if (!buffer)
        {
            env->CallVoidMethod(input, inputClose);
            env->CallVoidMethod(output, fileClose);
            env->CallVoidMethod(connection, disconnect);
            unlink(tempPath.c_str());
            if (uaName) env->DeleteLocalRef(uaName);
            if (uaValue) env->DeleteLocalRef(uaValue);
            env->DeleteLocalRef(input);
            env->DeleteLocalRef(output);
            env->DeleteLocalRef(connection);
            env->DeleteLocalRef(urlObject);
            env->DeleteLocalRef(jUrl);
            env->DeleteLocalRef(jPath);
            break;
        }

        success = true;

        while (true)
        {
            jint count =
                env->CallIntMethod(
                    input,
                    inputRead,
                    buffer,
                    0,
                    16 * 1024
                );

            if (env->ExceptionCheck())
            {
                env->ExceptionClear();
                success = false;
                break;
            }

            if (count < 0)
                break;

            if (count == 0)
                continue;

            env->CallVoidMethod(
                output,
                fileWrite,
                buffer,
                0,
                count
            );

            if (env->ExceptionCheck())
            {
                env->ExceptionClear();
                success = false;
                break;
            }
        }

        env->CallVoidMethod(input, inputClose);
        env->CallVoidMethod(output, fileClose);
        env->CallVoidMethod(connection, disconnect);

        if (env->ExceptionCheck())
        {
            env->ExceptionClear();
            success = false;
        }

        env->DeleteLocalRef(buffer);
        env->DeleteLocalRef(input);
        env->DeleteLocalRef(output);
        if (uaName) env->DeleteLocalRef(uaName);
        if (uaValue) env->DeleteLocalRef(uaValue);
        if (acceptName) env->DeleteLocalRef(acceptName);
        if (acceptValue) env->DeleteLocalRef(acceptValue);
        env->DeleteLocalRef(connection);
        env->DeleteLocalRef(urlObject);
        env->DeleteLocalRef(jUrl);
        env->DeleteLocalRef(jPath);

        if (!success)
        {
            unlink(tempPath.c_str());
            break;
        }

        // The caller only ever accepts a real PNG. If Imgur returned HTML
        // (error page/rate-limit/etc.), never cache it as the background.
        FILE* check = fopen(tempPath.c_str(), "rb");
        if (!check)
        {
            unlink(tempPath.c_str());
            success = false;
            break;
        }

        unsigned char signature[8] = {};
        size_t got = fread(signature, 1, sizeof(signature), check);
        fclose(check);

        const unsigned char pngSignature[8] =
            { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

        if (got != sizeof(pngSignature) ||
            memcmp(signature, pngSignature, sizeof(pngSignature)) != 0)
        {
            unlink(tempPath.c_str());
            success = false;
            break;
        }

        unlink(outputPath);
        if (rename(tempPath.c_str(), outputPath) != 0)
        {
            unlink(tempPath.c_str());
            success = false;
            break;
        }

    } while (false);

    if (attached)
        vm->DetachCurrentThread();

    if (artHandle)
        dlclose(artHandle);

    return success;
}

static std::string getRemoteImagePath(const char* fileName)
{
    std::string packageName = getProcessPackageName();
    return "/data/data/" + packageName + "/cache/" + fileName;
}

static bool remoteFileExists(const std::string& path)
{
    struct stat st{};
    return !path.empty() && stat(path.c_str(), &st) == 0 && st.st_size > 8;
}

static bool remotePngIsValid(const std::string& path)
{
    FILE* f = fopen(path.c_str(), "rb");
    if (!f)
        return false;

    unsigned char signature[8] = {};
    size_t got = fread(signature, 1, sizeof(signature), f);
    fclose(f);

    const unsigned char pngSignature[8] =
        { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

    return got == sizeof(pngSignature) &&
           memcmp(signature, pngSignature, sizeof(pngSignature)) == 0;
}

static void startPlayBackgroundDownload()
{
    int state = gPlayBackgroundDownloadState.load();

    // Do not get stuck forever after one failed Imgur request.
    if (state == -1)
    {
        gPlayBackgroundDownloadState.store(0);
        state = 0;
    }

    int expected = 0;
    if (!gPlayBackgroundDownloadState.compare_exchange_strong(expected, 1))
        return;

    std::string path =
        getRemoteImagePath("stumblecrowns_play_bg.png");

    if (remoteFileExists(path) && remotePngIsValid(path))
    {
        gPlayBackgroundDownloadState.store(2);
        return;
    }

    unlink(path.c_str());

    std::thread([path]()
    {
        bool ok =
            downloadFileWithAndroidJNI(
                kRemotePlayBackgroundUrl,
                path.c_str()
            );

        if (ok && remotePngIsValid(path))
            gPlayBackgroundDownloadState.store(2);
        else
        {
            unlink(path.c_str());
            gPlayBackgroundDownloadState.store(-1);
        }
    }).detach();
}

static void startLoginBackgroundDownload()
{
    int state = gLoginBackgroundDownloadState.load();

    if (state == -1)
    {
        gLoginBackgroundDownloadState.store(0);
        state = 0;
    }

    int expected = 0;
    if (!gLoginBackgroundDownloadState.compare_exchange_strong(expected, 1))
        return;

    std::string path =
        getRemoteImagePath("stumblecrowns_login_bg_v2.png");

    if (remoteFileExists(path) && remotePngIsValid(path))
    {
        gLoginBackgroundDownloadState.store(2);
        return;
    }

    unlink(path.c_str());

    std::thread([path]()
    {
        bool ok =
            downloadFileWithAndroidJNI(
                kRemoteLoginBackgroundUrl,
                path.c_str()
            );

        if (ok && remotePngIsValid(path))
            gLoginBackgroundDownloadState.store(2);
        else
        {
            unlink(path.c_str());
            gLoginBackgroundDownloadState.store(-1);
        }
    }).detach();
}

static void fitBackgroundToParent(void* bgTransform)
{
    if (!bgTransform || !objectGetComponent)
        return;

    void* bgGO =
        componentGetGameObject
            ? componentGetGameObject(bgTransform)
            : nullptr;

    if (!bgGO)
        return;

    void* rect =
        objectGetComponent(
            bgGO,
            String::Create("RectTransform")
        );

    if (!rect)
        return;

    // Full-bleed UI background: stretch to the complete parent.
    if (rectTransformSetAnchorMin)
        rectTransformSetAnchorMin(rect, Vector2{0.0f, 0.0f});

    if (rectTransformSetAnchorMax)
        rectTransformSetAnchorMax(rect, Vector2{1.0f, 1.0f});

    if (rectTransformSetAnchoredPosition)
        rectTransformSetAnchoredPosition(rect, Vector2{0.0f, 0.0f});

    if (rectTransformSetSizeDelta)
        rectTransformSetSizeDelta(rect, Vector2{0.0f, 0.0f});
}

static void updatePlayBackground(void* playViewInstance)
{
    if (!playViewInstance || !gameObjectFind ||
        !getTransform || !transformFind || !objectGetComponent)
        return;

    if (gPlayBackgroundDownloadState.load() != 2)
        return;

    if (!gPlayBackgroundSprite)
    {
        std::string path =
            getRemoteImagePath("stumblecrowns_play_bg.png");

        gPlayBackgroundSprite =
            loadLocalSprite(const_cast<char*>(path.c_str()));

        if (!gPlayBackgroundSprite)
        {
            // A stale/unsupported cached file must be downloaded again.
            unlink(path.c_str());
            gPlayBackgroundDownloadState.store(-1);
            return;
        }
    }

    // PlayViewController::_lobbyBackground is the real serialized Image field
    // at offset 0x70. Using it is much more reliable than guessing a scene path.
    void* lobbyBackground =
        *(void**)((uintptr_t)playViewInstance + 0x70);

    if (lobbyBackground)
    {
        if (imageSetPreserveAspect)
            imageSetPreserveAspect(lobbyBackground, false);

        // UnityEngine.UI.Image.Type.Simple = 0
        if (imageSetType)
            imageSetType(lobbyBackground, 0);

        if (imageSetSprite)
            imageSetSprite(lobbyBackground, gPlayBackgroundSprite);
        else
            *(void**)((uintptr_t)lobbyBackground + 0xD0) =
                gPlayBackgroundSprite;

        if (componentGetTransform)
            fitBackgroundToParent(componentGetTransform(lobbyBackground));

        return;
    }

    void* rootTransform = getTransform(playViewInstance);
    if (!rootTransform)
        return;

    void* bgTransform =
        transformFind(
            rootTransform,
            String::Create("BackgroundCanvas/Root/BG")
        );

    if (!bgTransform)
    {
        void* bgGO =
            gameObjectFind(
                String::Create("BackgroundCanvas/Root/BG")
            );

        if (bgGO)
            bgTransform = getTransform(bgGO);
    }

    if (!bgTransform)
    {
        void* bgGO = gameObjectFind(String::Create("BG"));
        if (bgGO)
            bgTransform = getTransform(bgGO);
    }

    if (!bgTransform)
        return;

    void* bgGO =
        componentGetGameObject
            ? componentGetGameObject(bgTransform)
            : nullptr;

    void* image =
        objectGetComponent(
            bgGO ? bgGO : bgTransform,
            String::Create("Image")
        );

    if (!image)
        return;

    if (imageSetPreserveAspect)
        imageSetPreserveAspect(image, false);

    if (imageSetSprite)
        imageSetSprite(image, gPlayBackgroundSprite);
    else
        *(void**)((uintptr_t)image + 0xD0) =
            gPlayBackgroundSprite;

    fitBackgroundToParent(bgTransform);
}

static void updateLoginBackground(void* loginViewInstance)
{
    if (!gameObjectFind || !objectGetComponent)
        return;

    if (gLoginBackgroundDownloadState.load() != 2)
        return;

    if (!gLoginBackgroundSprite)
    {
        std::string path =
            getRemoteImagePath("stumblecrowns_login_bg_v2.png");

        gLoginBackgroundSprite =
            loadLocalSprite(const_cast<char*>(path.c_str()));

        if (!gLoginBackgroundSprite)
        {
            unlink(path.c_str());
            gLoginBackgroundDownloadState.store(-1);
            return;
        }
    }

    void* novaBg =
        gameObjectFind(String::Create("nova-bg.png"));

    if (novaBg && gameObjectSetActive)
        gameObjectSetActive(novaBg, false);

    void* bg = nullptr;

    if (loginViewInstance && getTransform && transformFind)
    {
        void* root = getTransform(loginViewInstance);
        if (root)
        {
            bg = transformFind(
                root,
                String::Create("ViewArea/BG")
            );

            if (!bg)
            {
                void* viewArea =
                    transformFind(root, String::Create("ViewArea"));

                if (viewArea)
                    bg = transformFind(
                        viewArea,
                        String::Create("BG")
                    );
            }
        }
    }

    if (!bg)
    {
        void* viewArea =
            gameObjectFind(String::Create("ViewArea"));

        if (viewArea && getTransform && transformFind)
        {
            void* viewAreaTransform = getTransform(viewArea);
            if (viewAreaTransform)
                bg = transformFind(
                    viewAreaTransform,
                    String::Create("BG")
                );
        }
    }

    if (!bg)
        bg = gameObjectFind(String::Create("ViewArea/BG"));

    if (!bg)
        bg = gameObjectFind(
            String::Create(
                "LoginViewController(Clone)/ViewArea/BG"
            )
        );

    if (bg)
    {
        void* bgGO = bg;

        if (componentGetGameObject)
        {
            void* possibleGO = componentGetGameObject(bg);
            if (possibleGO)
                bgGO = possibleGO;
        }

        void* image =
            objectGetComponent(
                bgGO,
                String::Create("Image")
            );

        if (image)
        {
            if (imageSetPreserveAspect)
                imageSetPreserveAspect(image, false);

            if (imageSetSprite)
                imageSetSprite(image, gLoginBackgroundSprite);
            else
                *(void**)((uintptr_t)image + 0xD0) =
                    gLoginBackgroundSprite;

            fitBackgroundToParent(
                getTransform ? getTransform(bgGO) : nullptr
            );
            return;
        }
    }

    void* logoBg =
        gameObjectFind(String::Create("LogoBG (1)"));

    if (logoBg)
    {
        void* image =
            objectGetComponent(
                logoBg,
                String::Create("Image")
            );

        if (image)
        {
            if (imageSetPreserveAspect)
                imageSetPreserveAspect(image, false);

            if (imageSetSprite)
                imageSetSprite(image, gLoginBackgroundSprite);
            else
                *(void**)((uintptr_t)image + 0xD0) =
                    gLoginBackgroundSprite;

            fitBackgroundToParent(
                getTransform ? getTransform(logoBg) : nullptr
            );
        }
    }
}

static void serviceRemoteBackgrounds()
{
    if (gCurrentPlayViewInstance)
        updatePlayBackground(gCurrentPlayViewInstance);

    if (gCurrentLoginViewInstance)
        updateLoginBackground(gCurrentLoginViewInstance);
}



static void* loadLocalSprite(char* path) {
    if (!path || !loadTextureFromFile || !createSprite ||
        !textureGetWidth || !textureGetHeight) {
        return nullptr;
    }

    String* filePath = String::Create(path);
    if (!filePath) {
        return nullptr;
    }

    void* texture = loadTextureFromFile(filePath, false);
    if (!texture) {
        return nullptr;
    }

    int width = textureGetWidth(texture);
    int height = textureGetHeight(texture);
    if (width <= 0 || height <= 0) {
        return nullptr;
    }

    Rect rect = {
        0.0f,
        0.0f,
        static_cast<float>(width),
        static_cast<float>(height)
    };
    Vector2 pivot = {0.5f, 0.5f};

    return createSprite(texture, rect, pivot);
}

static void addCustomLevel(
    void* levelManager,
    const char* uniqueId,
    const char* friendlyName,
    const char* levelArt,
    const char* sceneName,
    const char* leftPanelDescription,
    const char* introObjective)
{
    if (!levelManager || !getType ||
        !scriptableObjectCreateInstance || !levelEntry)
        return;

    void* levelDefType =
        getType(String::Create("LevelDef, Assembly-CSharp"));

    if (!levelDefType)
        return;

    void* levelDef =
        scriptableObjectCreateInstance(levelDefType);

    if (!levelDef)
        return;

    *(String**)((uintptr_t)levelDef + 0x18) =
        String::Create(uniqueId);

    *(String**)((uintptr_t)levelDef + 0x20) =
        String::Create(friendlyName);

    *(String**)((uintptr_t)levelDef + 0x28) =
        String::Create(sceneName);

    *(String**)((uintptr_t)levelDef + 0x30) =
        String::Create(levelArt);

    *(String**)((uintptr_t)levelDef + 0x38) =
        String::Create(leftPanelDescription);

    *(String**)((uintptr_t)levelDef + 0x40) =
        String::Create(introObjective);

    levelEntry(levelManager, levelDef);
}

static void initializeCustomPartyLevels(void* levelManager)
{
    if (!levelManager || addedMap)
        return;

    addCustomLevel(levelManager,
                   "level19_respawnblock",
                   "Beast Dash",
                   "respawndash",
                   "Respawn Dash",
                   "SG sharkz",
                   "SG sharkz");

    addCustomLevel(levelManager,
                   "level15_respawnlaser",
                   "Beast Laser",
                   "respawnlaser",
                   "Respawn Laser",
                   "SG sharkz",
                   "SG sharkz");

    addCustomLevel(levelManager,
                   "eventlevel13_block_respawnlegendary",
                   "Beast Legendary",
                   "respawnlegendary",
                   "Respawn Legendary",
                   "SG sharkz",
                   "SG sharkz");

    addCustomLevel(levelManager,
                   "eventlevel1_respawndash",
                   "Beast Laser Dash",
                   "respawnlaserdash",
                   "Respawn Laser Dash",
                   "SG sharkz",
                   "SG sharkz");

    addCustomLevel(levelManager,
                   "eventlevel8_block_respawnendless",
                   "Beast Endless",
                   "respawnendless",
                   "Respawn Endless",
                   "SG sharkz",
                   "SG sharkz");

    addCustomLevel(levelManager,
                   "L_042_RespawnChicken",
                   "Beast Other Side",
                   "respawnchicken",
                   "Respawn Other Side",
                   "SG sharkz",
                   "SG sharkz");

    addCustomLevel(levelManager,
                   "eventlevel13_block_infinitelegendary",
                   "Block Dash Legendary Endless",
                   "infinitelegendary",
                   "Infinite Legendary",
                   "SG sharkz",
                   "SG sharkz");

    addCustomLevel(levelManager,
                   "level15_infinitelaser",
                   "Laser Tracer Endless",
                   "infinitelaser",
                   "Infinite Laser",
                   "SG sharkz",
                   "SG sharkz");

    addCustomLevel(levelManager,
                   "eventlevel1_infinitedash",
                   "Laser Dash Endless",
                   "infinitelaserdash",
                   "Infinite Laser Dash",
                   "SG sharkz",
                   "SG sharkz");

    addedMap = true;
}

void* openPopupFunc(char* name, char* header, char* content) {
    void* popup = nullptr;
    void* controllerInstance = findObject(getType(String::Create("PopupManager, Assembly-CSharp")));
    if (controllerInstance) {
        popup = openPopup(controllerInstance, String::Create(name), 0.0f, false);
        if (setPopupItems && popup) {
            setPopupItems(popup, String::Create(header), String::Create(content));
        }
    }
    return popup;
}

char* colorToHex(float r, float g, float b) {
    static char hex[8];
    int ri = (int)(r * 255);
    int gi = (int)(g * 255);
    int bi = (int)(b * 255);
    ri = std::max(0, std::min(255, ri));
    gi = std::max(0, std::min(255, gi));
    bi = std::max(0, std::min(255, bi));
    snprintf(hex, sizeof(hex), "%02X%02X%02X", ri, gi, bi);
    return hex;
}

void* animationThread(void*) {
    hueOffset = 0.0f;
    const char* text = "Stumble Sharkz 1.8 Mob";

    while (true) {
        usleep(75000);
        hueOffset += 0.02f;
        if (hueOffset > 1.0f) hueOffset -= 1.0f;

        char finalText[2048] = "";
        for (int i = 0; i < strlen(text); i++) {
            float t = (sinf(hueOffset * M_PI * 2.0f + i * 0.3f) + 1.0f) / 2.0f;

            float lightCyanR = 0.2f, lightCyanG = 0.6f, lightCyanB = 0.6f;
            float darkCyanR = 0.0f, darkCyanG = 0.3f, darkCyanB = 0.3f;
            
            float r = lightCyanR + (darkCyanR - lightCyanR) * t;
            float g = lightCyanG + (darkCyanG - lightCyanG) * t;
            float b = lightCyanB + (darkCyanB - lightCyanB) * t;

            r = std::max(0.0f, std::min(1.0f, r));
            g = std::max(0.0f, std::min(1.0f, g));
            b = std::max(0.0f, std::min(1.0f, b));

            char charText[256];
            snprintf(charText, sizeof(charText), "<color=#%s>%c</color>", colorToHex(r, g, b), text[i]);
            strcat(finalText, charText);
        }

        String* animatedText = String::Create(finalText);
        if (animatedText && tmpTextMeshPro) {
            *(String**)((uintptr_t)tmpTextMeshPro + 0xD0) = animatedText;
        }
    }
    
    isAnimating = true;
    return nullptr;
}



void (*oldPlayViewController)(void* instance);
void playViewController(void* instance) {
    if (oldPlayViewController)
        oldPlayViewController(instance);

    // Remember the live Unity instance. The download is asynchronous;
    // serviceRemoteBackgrounds() will apply the sprite as soon as it is ready.
    gCurrentPlayViewInstance = instance;
    startPlayBackgroundDownload();
    updatePlayBackground(instance);
    
    void* gameObject = gameObjectFind(String::Create("PassButton"));
    if (gameObject) {
        gameObjectSetActive(gameObject, false);
    }

    void* gameObject2 = gameObjectFind(String::Create("NewBadge"));
    if (gameObject2) {
        gameObjectSetActive(gameObject2, false);
    }
    
     void* tournamentXButton = gameObjectFind(String::Create("PLAY_VIEW_TournamentXButton"));
    if (tournamentXButton && getTransform && transformTranslate) {
        transformTranslate(getTransform(tournamentXButton), 0.0f, -160.0f, 0.0f);
    }

    void* header = gameObjectFind(String::Create("HeaderLobby"));
    if (header) {
        void* image = objectGetComponent(header, String::Create("Image"));
        if (image) {
            void* color = (void* (*))((uintptr_t)image + 0x20);
            if (color) {
                float* a = (float*)((uintptr_t)color + 0xC);
                if (a) {
                    *a = 0.0f;
                }
            }
        }
    }

    if (!addedMap) {
        void* inicializerType = getType(String::Create("Stumble.Initializer, Assembly-CSharp"));
        void* inicializerInstance = findObject(inicializerType);
        if (inicializerInstance) {
            void* levelManager = *(void**)((uintptr_t)inicializerInstance + 0x120);
            if (levelManager) {
                void* levelGroupListType = getType(String::Create("System.Collections.Generic.List`1[[LevelGroupDef, Assembly-CSharp]], mscorlib"));
                if (levelGroupListType) {
                    void* levelGroupList = activatorCreateInstance(levelGroupListType);
                    if (levelGroupList) {
                        void* levelDefType = getType(String::Create("LevelDef, Assembly-CSharp"));
                        if (levelDefType) {
                            void* levelDef1 = scriptableObjectCreateInstance(levelDefType);
                            if (levelDef1) {
                                *(String**)((uintptr_t)levelDef1 + 0x18) = String::Create("CustomLevel1");
                                *(String**)((uintptr_t)levelDef1 + 0x20) = String::Create("Sharkz Dash");
                                *(String**)((uintptr_t)levelDef1 + 0x28) = String::Create("Respawn Dash");
                                *(String**)((uintptr_t)levelDef1 + 0x30) = String::Create("Level19");
                                *(String**)((uintptr_t)levelDef1 + 0x38) = String::Create(".GG/SGSHARKZ");
                                *(String**)((uintptr_t)levelDef1 + 0x40) = String::Create(".GG/SGSHARKZ");
                                *(void**)((uintptr_t)levelDef1 + 0x48) = levelGroupList;
                                levelEntry(levelManager, levelDef1);

                                void* levelDef2 = scriptableObjectCreateInstance(levelDefType);
                                if (levelDef2) {
                                    *(String**)((uintptr_t)levelDef2 + 0x18) = String::Create("CustomLevel2");
                                    *(String**)((uintptr_t)levelDef2 + 0x20) = String::Create("Core Legendary");
                                    *(String**)((uintptr_t)levelDef2 + 0x28) = String::Create("Respawn Legendary");
                                    *(String**)((uintptr_t)levelDef2 + 0x30) = String::Create("eventlevel13_block_legendary");
                                    *(String**)((uintptr_t)levelDef2 + 0x38) = String::Create(".GG/SGCORE");
                                    *(String**)((uintptr_t)levelDef2 + 0x40) = String::Create(".GG/SGCORE");
                                    *(void**)((uintptr_t)levelDef2 + 0x48) = levelGroupList;
                              
									
                                }
                                addedMap = true;
                            }
                        }
                    }
                }
            }
        }
    }
    }

void (*oldHeaderViewHelper)(void* instance);
void headerViewHelper(void* instance) {
    oldHeaderViewHelper(instance);
    inTourx = false;
    void* text = *(void**)((uintptr_t)instance + 0x18);
    if (text) {
        *(String**)((uintptr_t)text + 0xD0) = String::Create("Stumble Sharkz 1.8 Mob");
    }
}

void (*oldHeaderLobbyViewHelper)(void* instance);
void headerLobbyViewHelper(void* instance) {
    oldHeaderLobbyViewHelper(instance);
    void* text = *(void**)((uintptr_t)instance + 0x38);
    if (text) {
        tmpTextMeshPro = text;
        *(String**)((uintptr_t)text + 0xD0) = String::Create("Stumble Sharkz 1.8 Mob");
    }
}

 



bool (*old_isVersionMinimium)(void* instance, void* client);
bool isVersionMinimium(void* instance, void* client)
{
    return true;
}

void showWelcomePopup() {
    if (!findObject || !getType || !openPopup || !setPopupItems || !componentGetTransform)
        return;

    void* popupManagerType = getType(String::Create("PopupManager, Assembly-CSharp"));
    if (!popupManagerType)
        return;

    void* popupManager = findObject(popupManagerType);
    if (!popupManager)
        return;

    void* popup = openPopup(popupManager, String::Create("InfoPopup"), 0.0f, false);
    if (!popup)
        return;

    String* username = String::Create("Player");
    if (userGetMe && userGetUsername) {
        void* me = userGetMe();
        if (me) {
            String* value = userGetUsername(me);
            if (value)
                username = value;
        }
    }

    std::string content =
        "<align=center><size=30>Welcome " + std::string(username ? username->getChars() : "Player") +
        "<#fff> to <#8000ff>StumbleSharkz <size=30><size=20><#fff>Here you can play old Stumble<color=orange> Guys<#fff> 0.64 With old Cassic <#ff0>Tournaments<#fff> and much more. Have fun playing!\n\n<align=center><size=23><color=white>Made by Noah";

    setPopupItems(popup, String::Create("Welcome to StumbleSharkz"), String::Create(content.c_str()));

    void* popupTransform = componentGetTransform(popup);
    if (!popupTransform)
        return;

    if (objectGetComponent) {
        void* rect = objectGetComponent(componentGetGameObject ? componentGetGameObject(popupTransform) : nullptr, String::Create("RectTransform"));
        if (rect && rectTransformSetSizeDelta)
            rectTransformSetSizeDelta(rect, Vector2{678.0f, 470.0f});
    }

    if (!transformFind || !componentGetGameObject || !objectGetComponent)
        return;

    void* buttonTransform = transformFind(popupTransform, String::Create("Button"));
    if (!buttonTransform)
        return;

    void* buttonObject = componentGetGameObject(buttonTransform);
    if (!buttonObject)
        return;

    void* buttonRect = objectGetComponent(buttonObject, String::Create("RectTransform"));
    if (buttonRect && rectTransformGetAnchorMin && rectTransformSetAnchorMin &&
        rectTransformGetAnchorMax && rectTransformSetAnchorMax &&
        rectTransformGetAnchoredPosition && rectTransformSetAnchoredPosition) {
        Vector2 anchorMin = rectTransformGetAnchorMin(buttonRect);
        Vector2 anchorMax = rectTransformGetAnchorMax(buttonRect);
        Vector2 anchoredPosition = rectTransformGetAnchoredPosition(buttonRect);
        anchorMin.x = 0.5f;
        anchorMax.x = 0.5f;
        anchoredPosition.x = 0.0f;
        rectTransformSetAnchorMin(buttonRect, anchorMin);
        rectTransformSetAnchorMax(buttonRect, anchorMax);
        rectTransformSetAnchoredPosition(buttonRect, anchoredPosition);
    }

    void* textTransform = transformFind(buttonTransform, String::Create("Text (TMP)"));
    if (!textTransform)
        return;

    void* textObject = componentGetGameObject(textTransform);
    if (!textObject)
        return;

    void* text = objectGetComponent(textObject, String::Create("TMPro.TextMeshProUGUI, Unity.TextMeshPro"));
    if (text && tmpSetText)
        tmpSetText(text, String::Create("Alright"));
}

// PlayViewController::Update() = RVA 0x11C4AD8.
// The image download is asynchronous, so applying the sprite only from Start()
// can miss the moment when the download finishes.
void (*oldPlayViewControllerUpdate)(void* instance);

void playViewControllerUpdate(void* instance)
{
    if (oldPlayViewControllerUpdate)
        oldPlayViewControllerUpdate(instance);

    if (instance)
    {
        gCurrentPlayViewInstance = instance;
        startPlayBackgroundDownload();
        updatePlayBackground(instance);
    }
}

void (*oldLoginViewController)(void* instance);
void loginViewController(void* instance) {
    if (oldLoginViewController)
        oldLoginViewController(instance);

    // Remember the live Unity instance. The download is asynchronous;
    // serviceRemoteBackgrounds() will apply the sprite when ready.
    gCurrentLoginViewInstance = instance;
    startLoginBackgroundDownload();
    updateLoginBackground(instance);

    showWelcomePopup();
}

void (*oldProfileViewController)(void* instance);
void profileViewController(void* instance) {
    oldProfileViewController(instance);
    
    void* original = gameObjectFind(String::Create("MenuStateRoot/PROFILE_VIEW(Clone)/Canvas/Root/GameObject/Right/SkinList/Viewport/ProfileDataRoot/NameArea/CrownDisplay"));
    if (original) {
        void* parentObj = gameObjectFind(String::Create("MenuStateRoot/PROFILE_VIEW(Clone)/Canvas/Root/GameObject/Right/SkinList/Viewport/ProfileDataRoot/NameArea"));
        void* parent = parentObj
            ? objectGetComponent(parentObj, String::Create("Transform"))
            : nullptr;

        void* idDisplay = parent ? objectInstantiate(original, parent) : nullptr;
        if (idDisplay) {
            objectSetName(idDisplay, String::Create("idDisplay"));
            gameObjectSetActive(idDisplay, true);
            void* transform = objectGetComponent(idDisplay, String::Create("Transform"));
            if (transform && transformTranslate) {
                transformTranslate(transform, -110.0f, -70.10f, -0.0f);
            }

            void* currencyIcon = gameObjectFind(String::Create("idDisplay/CurrencyIcon"));
            if (currencyIcon) {
                void* img = objectGetComponent(currencyIcon, String::Create("Image"));
                if (img) {
                    void* newSprite = loadLocalSprite(kProfileIdImagePath);
                    if (newSprite) {
                        *(void**)((uintptr_t)img + 0xD0) = newSprite;                  
                    }
                }
            }
  
            void* textT = getType(String::Create("TMPro.TextMeshProUGUI, Unity.TextMeshPro"));
            if (textT) {
                void* text = objectGetComponentInChildren(idDisplay, textT);
                if (text) {
                    char playerId[12];
                    snprintf(playerId, sizeof(playerId), "%d", getUserId());
                    *(String**)((uintptr_t)text + 0xD0) = String::Create(playerId);
                }
            }
        }
    }
}

void* (*old_backboneHttpClient)(void* instance, void* baseUri, String* applicationI);
void* backboneHttpClient(void* instance, void* baseUri, String* applicationId)
{
    if (!old_backboneHttpClient || !createUri)
        return nullptr;
    return old_backboneHttpClient(instance, createUri(String::Create("https://backbone-core-production-c200.up.railway.app/")), String::Create("CoreSG"));
}

void* (*old_backboneInitialize)(void* baseUri, String* applicationId);
void* backboneInitialize(void* baseUri, String* applicationId)
{
    if (!old_backboneInitialize || !createUri)
        return nullptr;
    return old_backboneInitialize(createUri(String::Create("https://backbone-core-production-c200.up.railway.app/")), applicationId);
}


void (*oldInitializer)(void* instance);
void initializer(void* instance) {
    void** runtimeConfigurationPtr = (void**)((uintptr_t)instance + 0x20);
    if (runtimeConfigurationPtr && *runtimeConfigurationPtr) {
        void* runtimeConfiguration = *runtimeConfigurationPtr;
        void** environmentConfigPtr = (void**)((uintptr_t)runtimeConfiguration + 0x18);
        if (environmentConfigPtr && *environmentConfigPtr) {
            void* environmentConfig = *environmentConfigPtr;
            String** backendHost = (String**)((uintptr_t)environmentConfig + 0x20);
            if (backendHost) {
                *backendHost = String::Create("https://stumblesharkz-production-b8ab.up.railway.app");
            }

            bool* displayEnviroment = (bool*)((uintptr_t)environmentConfig + 0x4A);
            if (displayEnviroment) {
                *displayEnviroment = true;
            }

            bool* allowTour = (bool*)((uintptr_t)environmentConfig + 0x30);
            if (allowTour) {
                *allowTour = true;
            }
        }
    }

    if (oldInitializer)
        oldInitializer(instance);
}    

void* (*oldUiController)(void* instance);

void* uiController(void* instance)
{
    serviceRemoteBackgrounds();

    if (instance && setCustomGamemode == 1)
    {
        // =========================================================
        // LEAVE BUTTON
        // UIController::_uiLeaveButtons = 0xA0
        // =========================================================

        void* uiLeaveButtons =
            *(void**)((uintptr_t)instance + 0xA0);

        if (uiLeaveButtons)
        {
            // -----------------------------------------------------
            // UILeaveButtons::SetLeaveRootActive(true)
            // RVA: 0x14D8128
            // -----------------------------------------------------

            using SetLeaveRootActive_t = void (*)(void*, bool);

SetLeaveRootActive_t SetLeaveRootActive =
    (SetLeaveRootActive_t)getAbsoluteAddress(
        targetLibName,
        0x14D8128
    );


using SetLeaveButtonPosition_t =
    void (*)(void*, Vector2, Vector2, Vector2);

SetLeaveButtonPosition_t SetLeaveButtonPosition =
    (SetLeaveButtonPosition_t)getAbsoluteAddress(
        targetLibName,
        0x14D7D40
    );

            if (SetLeaveButtonPosition)
            {
                Vector2 anchorMin;
                Vector2 anchorMax;
                Vector2 position;

                // Oben links verankern
                anchorMin.x = 0.0f;
                anchorMin.y = 1.0f;

                anchorMax.x = 0.0f;
                anchorMax.y = 1.0f;

                // Unterhalb vom Ping
                position.x = 95.0f;
                position.y = -200.0f;

                SetLeaveButtonPosition(
                    uiLeaveButtons,
                    anchorMin,
                    anchorMax,
                    position
                );
            }

            // -----------------------------------------------------
            // LeaveRoot aus UILeaveButtons
            // UILeaveButtons::_leaveRoot = 0x18
            // -----------------------------------------------------

            void* leaveRoot =
                *(void**)((uintptr_t)uiLeaveButtons + 0x18);

            if (leaveRoot)
            {
                gameObjectSetActive(
                    leaveRoot,
                    true
                );

                // -------------------------------------------------
                // TextMeshProUGUI suchen
                // -------------------------------------------------

                void* leaveText =
                    objectGetComponent(
                        leaveRoot,
                        String::Create("TextMeshProUGUI")
                    );

                if (leaveText)
                {
                    // "Salir" -> "Leave"
                    *(String**)((uintptr_t)leaveText + 0xD0) =
                        String::Create("Leave");

                    float* fontSize =
                        (float*)((uintptr_t)leaveText + 0x1DC);

                    if (fontSize)
                    {
                        *fontSize = 30.0f;
                    }
                }
            }
        }
    }
    
    
    
 
void* playerNameGO =
    gameObjectFind(
        String::Create("PlayerName(Clone)")
    );

    if (playerNameGO)
    {
        void* playerNamesGO =
            gameObjectFind(
                String::Create("PlayerNames")
            );

        void* playerNameParent = nullptr;

        if (playerNamesGO)
        {
            playerNameParent =
                objectGetComponent(
                    playerNamesGO,
                    String::Create("Transform")
                );
        }

        if (playerNameParent)
        {
            void* beastWatermark =
                objectInstantiate(
                    playerNameGO,
                    playerNameParent
                );

            if (beastWatermark)
            {
                objectSetName(
                    beastWatermark,
                    String::Create("Core Watermark")
                );

                gameObjectSetActive(
                    beastWatermark,
                    true
                );

                void* beastWatermarkTransform =
                    objectGetComponent(
                        beastWatermark,
                        String::Create("Transform")
                    );

                if (beastWatermarkTransform)
                {
                    transformTranslate(
                        beastWatermarkTransform,
                        0.0f,
                        70.0f,
                        0.0f
                    );
                }

                void* text =
                    objectGetComponent(
                        beastWatermark,
                        String::Create("TextMeshProUGUI")
                    );

                if (text)
                {
                    *(String**)((uintptr_t)text + 0xD0) =
                        String::Create("Stumble Sharkz");

                    float* fontSize =
                        (float*)((uintptr_t)text + 0x1DC);

                    if (fontSize)
                    {
                        *fontSize = 30.0f;
                    }
                }
            }
        }

        
        
        

        void* arrowTransform =
            gameObjectFind(
                String::Create(
                    "UICamera/UI/NonPersistentRoot/"
                    "GameHUD/PlayerHUDViews/"
                    "PlayerHUDView(Clone)/Container/"
                    "PlayerNames/Beast Watermark/Arrow"
                )
            );

        if (arrowTransform)
        {
            gameObjectSetActive(
                arrowTransform,
                false
            );
        }
    }

    return oldUiController(instance);
}

String* (*oldGetBotName)(void* instance);
String* getBotName(void* instance) {
    srand((unsigned)time(nullptr) ^ (unsigned)pthread_self());
    int botId = 1000 + (rand() % 9000);
    char botIdChar[32];
    snprintf(botIdChar, sizeof(botIdChar), "%d", botId);
    
    char fullText[32];
    snprintf(fullText, sizeof(fullText), "sharkzbot<sup><#ffff00>%s", botIdChar);
    return String::Create(fullText);
}

bool (*originalIsUpdateAvailable)(void* instance);
bool isUpdateAvailable(void* instance) {
    return false;
}

void (*oldLobbyPopup)(void* instance);
void lobbyPopup(void* instance) {
}

void (*oldInicialize)(void* instance);
void inicialize(void* instance) {
}

void (*oldSend)(void* instance);
void asend(void* instance) {
}

bool (*oldSpecialEmoteFilter)(void* instance);
bool specialEmoteFilter(void* instance) {
    return true;
}

bool (*oldValidateTourxData)(void* instance);
bool validateTourxData(void* instance) {
    return true;
}

bool (*oldCustomParty)(void* instance);
bool customParty(void* instance) {
    return true;
}

bool (*oldTourx)(void* instance);
bool tourx(void* instance) {
    return true;
}

bool (*oldTourxMeta)(void* instance);
bool tourxMeta(void* instance) {
    return true;
}

bool (*oldCreatorcode)(void* instance);
bool creatorcode(void* instance) {
    return true;
}

bool (*oldUnlockCosmetic)(void* instance);
bool unlockCosmetic(void* instance) {
    return true;
}

bool (*originalIsUpdateRequired)(void* instance);
bool isUpdateRequired(void* instance) {
    return false;
}

void (*oldJoinTourx)(void* instance, void* tournament);
void joinTourx(void* instance, void* tournament) {
    oldJoinTourx(instance, tournament);
    inTourx = true;
}

void (*oldLeaveTourx)(void* instance, void* tournament);
void leaveTourx(void* instance, void* tournament) {
    oldLeaveTourx(instance, tournament);
    inTourx = false;
}

void (*oldRuntimeConfig)(void* instance, void* stream);
void runtimeConfig(void* instance, void* stream) {
    if (instance) {
        int* botCount = (int*)((uintptr_t)instance + 0x68);
        if (botCount && inTourx) {
            *botCount = 0;
        }
    }
    oldRuntimeConfig(instance, stream);
}

bool (*oldOpAuthenticate)(void* instance, String* appId, String* appVersion, void* authValues, String* regionCode, bool getLobbyStatistics);
bool opAuthenticate(void* instance, String* appId, String* appVersion, void* authValues, String* regionCode, bool getLobbyStatistics) {
    if (authValues) {
        uint8_t* authTypeField = (uint8_t*)((uintptr_t)authValues + 0x10);
        *authTypeField = 255;

        String** userIdField = (String**)((uintptr_t)authValues + 0x30);
        if (userIdField) {
            int playerId = getUserId();
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", playerId);
            *userIdField = String::Create(buf);
        }
    }
    if (!oldOpAuthenticate)
        return false;
    return oldOpAuthenticate(instance, appId, appVersion, authValues, regionCode, getLobbyStatistics);
}

void* (*oldCloneAppSettings)(void* appSettings);
void* cloneAppSettings(void* appSettings) {
    void* result = oldCloneAppSettings(appSettings);
    if (result) {
        String** appIdRealtime = (String**)((uint8_t*)result + 0x10);
        if (appIdRealtime) {
            *appIdRealtime = String::Create("3e8a970f-12be-41fc-b8d0-93c657234f85");
        }
    }
    return result;
}

String* (*oldGetTranslation)(void* instance, String* key, bool fixForRtl, int maxLineLengthForRTL, bool ignoreRTLnumbers);
String* getTranslation(void* instance, String* key, bool fixForRtl, int maxLineLengthForRTL, bool ignoreRTLnumbers) {
    const char* keyStr = key->getChars();
    return oldGetTranslation(instance, key, fixForRtl, maxLineLengthForRTL, ignoreRTLnumbers);
}

void* (*oldMapConfigGetter)(void* instance);
void* mapConfigGetter(void* instance) {
    void* result = oldMapConfigGetter(instance);
    if (result && setCustomGamemode == 0) { 
        int* levelType = (int*)((uint8_t*)result + 0x78);
        if (levelType) {
            *levelType = 0;
        }
     }
    return result;
}

String* (*oldGetMapName)(void* instance);
String* getMapName(void* instance) {
    String* result = oldGetMapName(instance);
    if (!result)
        return result;
    const char* nameChars = result->getChars();

    if (nameChars) {
        if (strcmp(nameChars, String::Create("Respawn Dash")->getChars()) == 0) {
            setCustomGamemode = 1;
            return String::Create("level19_block");
        }
        if (strcmp(nameChars, String::Create("Respawn Legendary")->getChars()) == 0)
        {
            setCustomGamemode = 1;
        
            return String::Create("eventlevel13_block_legendary");
        }
		
		if (strcmp(nameChars, String::Create("Respawn Hour")->getChars()) == 0)
        {
            setCustomGamemode = 1;
        
            return String::Create("level24_streamtiles");
        }
        if (strcmp(nameChars, String::Create("Respawn Laser")->getChars()) == 0) {
            setCustomGamemode = 1;
            return String::Create("level15_laser");
        }
        if (strcmp(nameChars, String::Create("Respawn Laser Dash")->getChars()) == 0) {
            setCustomGamemode = 1;
            return String::Create("eventlevel1_dash");
        }
        if (strcmp(nameChars, String::Create("eventlevel8_block_endless")->getChars()) == 0 ||
            strcmp(nameChars, String::Create("Respawn Endless")->getChars()) == 0) {
            setCustomGamemode = 1;
            return String::Create("eventlevel8_block_endless");
        }
        if (strcmp(nameChars, String::Create("Respawn Other Side")->getChars()) == 0) {
            setCustomGamemode = 1;
            return String::Create("L_042_Chicken");
        }
        if (strcmp(nameChars, String::Create("Infinite Legendary")->getChars()) == 0) {
            setCustomGamemode = 1;
            return String::Create("eventlevel13_block_legendary");
        }
        if (strcmp(nameChars, String::Create("Infinite Laser")->getChars()) == 0) {
            setCustomGamemode = 1;
            return String::Create("level15_laser");
        }
        if (strcmp(nameChars, String::Create("Infinite Laser Dash")->getChars()) == 0) {
            setCustomGamemode = 1;
            return String::Create("eventlevel1_dash");
        }
    }
    setCustomGamemode = 0;
    return result;
}


void* hackThread(void*) {
    int attempts = 0;
    do {
        sleep(1);
        attempts++;
        if (attempts > 30) {
            return nullptr;
        }
    } while (!isLibraryLoaded(targetLibName));

    HOOK_LIB("libil2cpp.so", "0x136C188", initializer, oldInitializer); 
    HOOK_LIB("libil2cpp.so", "0x12C46D4", getTranslation, oldGetTranslation);
    HOOK_LIB("libil2cpp.so", "0x2D7DEF8", opAuthenticate, oldOpAuthenticate);
    HOOK_LIB("libil2cpp.so", "0x1373200", isUpdateAvailable, originalIsUpdateAvailable);
    HOOK_LIB("libil2cpp.so", "0x11C39CC", lobbyPopup, oldLobbyPopup);
    HOOK_LIB("libil2cpp.so", "0x1305C4C", unlockCosmetic, oldUnlockCosmetic);
    HOOK_LIB("libil2cpp.so", "0x14E5D28", customParty, oldCustomParty);
    HOOK_LIB("libil2cpp.so", "0x14E5304", creatorcode, oldCreatorcode);
    HOOK_LIB("libil2cpp.so", "0x149ED3C", tourxMeta, oldTourxMeta);
    HOOK_LIB("libil2cpp.so", "0x14998D8", tourx, oldTourx);
    HOOK_LIB("libil2cpp.so", "0x12BAF00", inicialize, oldInicialize);
    HOOK_LIB("libil2cpp.so", "0x12B8A74", asend, oldSend);
    HOOK_LIB("libil2cpp.so", "0x1495958", validateTourxData, oldValidateTourxData);
    HOOK_LIB("libil2cpp.so", "0x1373214", isUpdateRequired, originalIsUpdateRequired);
    HOOK_LIB("libil2cpp.so", "0x11B4188", cloneAppSettings, oldCloneAppSettings);
    HOOK_LIB("libil2cpp.so", "0x13E2520", specialEmoteFilter, oldSpecialEmoteFilter);
    HOOK_LIB("libil2cpp.so", "0x14A20AC", joinTourx, oldJoinTourx);
    HOOK_LIB("libil2cpp.so", "0x149E1CC", leaveTourx, oldLeaveTourx);
    HOOK_LIB("libil2cpp.so", "0x3D683FC", runtimeConfig, oldRuntimeConfig);
    HOOK_LIB("libil2cpp.so", "0x3D40A18", mapConfigGetter, oldMapConfigGetter);
    HOOK_LIB("libil2cpp.so", "0x14250A4", getMapName, oldGetMapName);
    HOOK_LIB("libil2cpp.so", "0x11C504C", playViewController, oldPlayViewController);
    HOOK_LIB("libil2cpp.so", "0x11C4AD8", playViewControllerUpdate, oldPlayViewControllerUpdate);
    HOOK_LIB("libil2cpp.so", "0x1569318", loginViewController, oldLoginViewController);
    HOOK_LIB("libil2cpp.so", "0x1221DE8", uiController, oldUiController);
    HOOK_LIB("libil2cpp.so", "0x11CACAC", profileViewController, oldProfileViewController);
    HOOK_LIB("libil2cpp.so", "0x155D710", headerViewHelper, oldHeaderViewHelper);
    HOOK_LIB("libil2cpp.so", "0x13C777C", login, oldLogin);
    HOOK_LIB("libil2cpp.so", "0x1438194", getBotName, oldGetBotName);
    HOOK_LIB("libil2cpp.so", "0x155DDB4", headerLobbyViewHelper, oldHeaderLobbyViewHelper);
    HOOK_LIB("libil2cpp.so", "0x15D8DC4", backboneHttpClient, old_backboneHttpClient);
    HOOK_LIB("libil2cpp.so", "0x15D8E08", backboneInitialize, old_backboneInitialize);
    HOOK_LIB("libil2cpp.so", "0x12C2638", isVersionMinimium, old_isVersionMinimium);
    levelManagerInstance = (void (*))getAbsoluteAddress(targetLibName, 0x15675A8);
	

    buildString = (String * (*)(void*, char*, int, int))getAbsoluteAddress(targetLibName, 0x3A64F70);
    createSprite = (void* (*)(void*, Rect, Vector2))getAbsoluteAddress(targetLibName, 0x3801E4C);
    gameObjectFind = (void* (*)(String*))getAbsoluteAddress(targetLibName, 0x37EA304);
    objectGetComponent = (void* (*)(void*, String*))getAbsoluteAddress(targetLibName, 0x37E9194);
    getTransform = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37E9AB8);
    getParent = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37FC6A8);
    vector3Ctor = (void* (*)(float, float, float))getAbsoluteAddress(targetLibName, 0x37F4A00);
    gameObjectSetActive = (void* (*)(void*, bool))getAbsoluteAddress(targetLibName, 0x37E9BF4);
    objectGetName = (String * (*)(void*))getAbsoluteAddress(targetLibName, 0x37EA87C);
    objectSetName = (void* (*)(void*, String*))getAbsoluteAddress(targetLibName, 0x37EA93C);
    scriptableObjectCreateInstance = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37ED120);
    objectInstantiate = (void* (*)(void*, void*))getAbsoluteAddress(targetLibName, 0x37EAFA4);
    objectDestroy = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37EB1F8);
    transformGetPosition = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37FBBA8);
    transformSetPosition = (void* (*)(void*, void*))getAbsoluteAddress(targetLibName, 0x37FBC48);
    componentSetActive = (void* (*)(void*, bool))getAbsoluteAddress(targetLibName, 0x37E9BB0);
    toVec = (void* (*)(void*, float, float, float))getAbsoluteAddress(targetLibName, 0x37FD7C4);
    objectGetComponentInChildren = (void* (*)(void*, void*))getAbsoluteAddress(targetLibName, 0x37E922C);
    transformTranslate = (void* (*)(void*, float, float, float))getAbsoluteAddress(targetLibName, 0x37FCD78);
    componentGetTransform = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37EF8A0);
    componentGetGameObject = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37EF8DC);
    transformFind = (void* (*)(void*, String*))getAbsoluteAddress(targetLibName, 0x37FDC98);
    rectTransformGetAnchorMin = (Vector2 (*)(void*))getAbsoluteAddress(targetLibName, 0x37FE6F4);
    rectTransformSetAnchorMin = (void (*)(void*, Vector2))getAbsoluteAddress(targetLibName, 0x37FE784);
    rectTransformGetAnchorMax = (Vector2 (*)(void*))getAbsoluteAddress(targetLibName, 0x37FE810);
    rectTransformSetAnchorMax = (void (*)(void*, Vector2))getAbsoluteAddress(targetLibName, 0x37FE8A0);
    rectTransformGetAnchoredPosition = (Vector2 (*)(void*))getAbsoluteAddress(targetLibName, 0x37FE92C);
    rectTransformSetAnchoredPosition = (void (*)(void*, Vector2))getAbsoluteAddress(targetLibName, 0x37FE9BC);
    rectTransformSetSizeDelta = (void (*)(void*, Vector2))getAbsoluteAddress(targetLibName, 0x37FEAD8);
    userGetMe = (void* (*)())getAbsoluteAddress(targetLibName, 0x13C9CC0);
    userGetUsername = (String* (*)(void*))getAbsoluteAddress(targetLibName, 0x13D2C50);
    tmpSetText = (void (*)(void*, String*))getAbsoluteAddress(targetLibName, 0x3722EEC);
    activatorCreateInstance = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x3C11CA8);
    findObject = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x37EB714);
    levelEntry = (void* (*)(void*, void*))getAbsoluteAddress(targetLibName, 0x15675A8);
    getTraducao = (String* (*)(String*))getAbsoluteAddress(targetLibName, 0x12C46D4);
    getType =  (void* (*)(String*))getAbsoluteAddress(targetLibName, 0x3C02250);
    createUri = (void* (*)(String*))getAbsoluteAddress(targetLibName, 0x34E68B8);
    cloneCustomRoomProperties = (void* (*)(void*))getAbsoluteAddress(targetLibName, 0x142DB84);
    getHashtableItem = (void* (*)(void*, String*))getAbsoluteAddress(targetLibName, 0x2D06CF8);
    openPopup = (void* (*)(void*, String*, float, bool))getAbsoluteAddress(targetLibName, 0x15832F0);
    setPopupItems = (void* (*)(void*, String*, String*))getAbsoluteAddress(targetLibName, 0x1580630);
    getCurrencySprite =  (void* (*)(void*, String*))getAbsoluteAddress(targetLibName, 0x122588C);
    imageConversionLoadImage =  (bool (*)(void*, void*, bool))getAbsoluteAddress(targetLibName, 0x382A5F0);
    loadTextureFromFile = (void* (*)(String*, bool))getAbsoluteAddress(targetLibName, 0x14B95D8);
    textureGetWidth = (int (*)(void*))getAbsoluteAddress(targetLibName, 0x37C4BEC);
    textureGetHeight = (int (*)(void*))getAbsoluteAddress(targetLibName, 0x37C4C68);

    // UnityEngine.UI.Image::set_sprite / set_preserveAspect
    imageSetSprite =
        (void (*)(void*, void*))getAbsoluteAddress(targetLibName, 0x38BE0B4);
    imageSetPreserveAspect =
        (void (*)(void*, bool))getAbsoluteAddress(targetLibName, 0x38CB8A4);
    imageSetType =
        (void (*)(void*, int))getAbsoluteAddress(targetLibName, 0x38BE3B4);
    actacteCreateInstanceParams = (void* (*)(void*, void*[]))getAbsoluteAddress(targetLibName, 0x3C11C78);
    
    return NULL;
}

__attribute__((constructor))
void libMain() {
    pthread_t ptid;
    pthread_create(&ptid, NULL, hackThread, NULL);
}






