/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <numeric>
#include <pthread.h>

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <binder/ProcessState.h>

#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>

#include <ui/DisplayInfo.h>
#include <ui/PixelFormat.h>

// TODO: Fix Skia.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <SkImageEncoder.h>
#include <SkData.h>
#pragma GCC diagnostic pop

using namespace android;

static uint32_t DEFAULT_DISPLAY_ID = ISurfaceComposer::eDisplayIdMain;

static void usage(const char* pname)
{
    fprintf(stderr,
            "usage: %s [-hpsD] [-r repeat_num] [-f minLayer] [-t maxLayer] [-d display-id] [-n filename]\n"
                    "   -h: this message\n"
                    "   -p: save the file as a png.\n"
                    "   -s: show screenshot info\n"
                    "   -D: do not output to file\n"
                    "   -r: repeat @repeat_num times\n"
                    "   -f: mini layer of this capture, default 0\n"
                    "   -t: max layer of this capture, default -1U\n"
                    "   -d: specify the display id to capture, default %d.\n"
                    "   -n: filename to save, in repeat mod, filename of the nth capture will be @filename-n\n"
                    "If filename is not given, the results will be printed to stdout.\n",
            pname, DEFAULT_DISPLAY_ID
    );
}

static SkColorType flinger2skia(PixelFormat f)
{
    switch (f) {
        case PIXEL_FORMAT_RGB_565:
            return kRGB_565_SkColorType;
        default:
            return kN32_SkColorType;
    }
}

/*
static status_t notifyMediaScanner(const char* fileName) {
    String8 cmd("am broadcast -a android.intent.action.MEDIA_SCANNER_SCAN_FILE -d file://");
    String8 fileUrl("\"");
    fileUrl.append(fileName);
    fileUrl.append("\"");
    cmd.append(fileName);
    cmd.append(" > /dev/null");
    int result = system(cmd.string());
    if (result < 0) {
        fprintf(stderr, "Unable to broadcast intent for media scanner.\n");
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}
 */

int main(int argc, char** argv)
{
    ProcessState::self()->startThreadPool();

    const char* pname = argv[0];
    const char* fn = NULL;
    bool png = false;
    bool show_info = false;
    bool dry_run = false;
    int32_t displayId = DEFAULT_DISPLAY_ID;
    int c;
    uint32_t repeat_num = 1;
    uint32_t minLayer = 0;
    uint32_t maxLayer = -1U;
    while ((c = getopt(argc, argv, "phsDd:r:f:t:n:")) != -1) {
        switch (c) {
            case 'D':
                dry_run = true;
                break;
            case 's':
                show_info = true;
                break;
            case 'p':
                png = true;
                break;
            case 'd':
                displayId = atoi(optarg);
                break;
            case 'r':
                repeat_num = atoi(optarg);
                break;
            case 'f':
                minLayer = atoi(optarg);
                break;
            case 't':
                maxLayer = atoi(optarg);
                break;
            case 'n':
                fn = optarg;
                break;
            case '?':
            case 'h':
                usage(pname);
                return 1;
        }
    }
    /*
    if (optind >= argc)
    {
        fprintf(stderr, "Expected argument after options\r\n");
        fprintf(stderr, "%d %d \r\n", optind, argc);
        return 1;
    }
     */
    /*
    argc -= optind;
    argv += optind;

    int fd = -1;
    const char* fn = NULL;
    if (argc == 0) {
        fd = dup(STDOUT_FILENO);
    } else if (argc == 1) {
        fn = argv[0];
        fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0664);
        if (fd == -1) {
            fprintf(stderr, "Error opening file: %s (%s)\n", fn, strerror(errno));
            return 1;
        }
        const int len = strlen(fn);
        if (len >= 4 && 0 == strcmp(fn+len-4, ".png")) {
            png = true;
        }
    }

    if (fd == -1) {
        usage(pname);
        return 1;
    }
    */

    void const* mapbase = MAP_FAILED;
    ssize_t mapsize = -1;

    void const* base = NULL;
    uint32_t w, s, h, f;
    size_t size = 0;

    // Maps orientations from DisplayInfo to ISurfaceComposer
    static const uint32_t ORIENTATION_MAP[] = {
        ISurfaceComposer::eRotateNone, // 0 == DISPLAY_ORIENTATION_0
        ISurfaceComposer::eRotate270, // 1 == DISPLAY_ORIENTATION_90
        ISurfaceComposer::eRotate180, // 2 == DISPLAY_ORIENTATION_180
        ISurfaceComposer::eRotate90, // 3 == DISPLAY_ORIENTATION_270
    };

    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(displayId);
    if (display == NULL) {
        fprintf(stderr, "Unable to get handle for display %d\n", displayId);
        return 1;
    }
    /*
    sp<IBinder> display = SurfaceComposerClient::createDisplay(String8("myDisplay"), false);
    if (display == NULL) {
        fprintf(stderr, "Unable to get handle for display \n");
        return 1;
    }
    SurfaceComposerClient::openGlobalTransaction();
    SurfaceComposerClient::setDisplaySize(display, 270, 480);
    SurfaceComposerClient::closeGlobalTransaction();
     */

    Vector<DisplayInfo> configs;
    SurfaceComposerClient::getDisplayConfigs(display, &configs);
    int activeConfig = SurfaceComposerClient::getActiveConfig(display);
    if (static_cast<size_t>(activeConfig) >= configs.size()) {
        fprintf(stderr, "Active config %d not inside configs (size %zu)\n",
                activeConfig, configs.size());
        return 1;
    }
    uint8_t displayOrientation = configs[activeConfig].orientation;
    uint32_t captureOrientation = ORIENTATION_MAP[displayOrientation];

    ScreenshotClient screenshot;
    uint32_t num = 0;
    int fd = -1;
    fprintf(stderr, "goo\r\n");
    //clock_t last = clock();
    auto last = std::chrono::system_clock::now();
    while(num < repeat_num)
    {
        //clock_t cur = clock();
        auto cur = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = cur - last;
        fprintf(stderr, "since last update %f\r\n", diff.count());
        //last = cur;

        auto begin = std::chrono::system_clock::now();
        status_t result = screenshot.update(display, Rect(), 0, 0, minLayer, maxLayer,
                                            false, captureOrientation);
        auto end = std::chrono::system_clock::now();
        diff = end - begin;
        fprintf(stderr, "cost of update = %f\r\n", diff.count());

        num++;
        if (result != NO_ERROR) {
            fprintf(stderr, "screenshot.update error at %dth capture : %d\r\n", num, result);
            //usleep(1000 * 10);
            //continue;
            return 1;
        }
        base = screenshot.getPixels();
        size = screenshot.getSize();
        w = screenshot.getWidth();
        h = screenshot.getHeight();
        s = screenshot.getStride();
        f = screenshot.getFormat();
        if (show_info)
        {
            show_info = false;  //only once
            fprintf(stderr, "capture info\n"
                    "w=%d\n"
                    "h=%d\n"
                    "stride=%d\n"
                    "format=%d\n"
                    "size=%d\n"
                    , w, h, s, f, (int)size);
        }
        if (fn == NULL)
            fd = dup(STDOUT_FILENO);
        else
        {
            char name[64];
            strncpy(name, fn, 32);
            if (png)
                sprintf(name, "%s-%06d.png", name, num);
            else
                sprintf(name, "%s-%06d", name, num);
            fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0664);
            if (fd == -1) {
                fprintf(stderr, "Error opening file: %s (%s)\n", name, strerror(errno));
                return 1;
            }
        }
        if (base != NULL)
        {
            if (png) {
                const SkImageInfo info = SkImageInfo::Make(w, h, flinger2skia(f),
                                                           kPremul_SkAlphaType);
                SkAutoTUnref<SkData> data(SkImageEncoder::EncodeData(info, base, s*bytesPerPixel(f),
                                                                     SkImageEncoder::kPNG_Type, SkImageEncoder::kDefaultQuality));
                if (data.get() && !dry_run) {
                    write(fd, data->data(), data->size());
                }
            }
            else if (!dry_run)
            {
                /*
                write(fd, &w, 4);
                write(fd, &h, 4);
                write(fd, &f, 4);
                 */
                size_t Bpp = bytesPerPixel(f);
                char *buf = new char[h * w * Bpp];
                auto begin = std::chrono::system_clock::now();
                /*
                for (size_t y=0 ; y<h ; y++) {
                    write(fd, base, w*Bpp);
                    base = (void *)((char *)base + s*Bpp);
                }
                 */
                size_t offset = 0;
                for (size_t i = 0; i < h; ++i) {
                    memcpy(buf + offset, base, w * Bpp);
                    base = (void *)((char *)base + s*Bpp);
                    offset += w * Bpp;
                }
                write(fd, buf, h * w * Bpp);
                delete [] buf;

                auto end = std::chrono::system_clock::now();
                std::chrono::duration<double> diff = end - begin;
                fprintf(stderr, "cost of write raw= %f\r\n", diff.count());
                //write(fd, base, size);
            }
        }
        if (fn != NULL)
            close(fd);
    }

    /*
    uint32_t minLayerZ = 10000;
    uint32_t maxLayerZ = 100000;
    fprintf(stdout, "mini layer %d \r\n", minLayerZ);
    fprintf(stdout, "max layer %d \r\n", maxLayerZ);
    status_t result = screenshot.update(display, Rect(), 0, 0, minLayerZ, maxLayerZ,
            false, captureOrientation);
    if (result == NO_ERROR) {
        base = screenshot.getPixels();
        w = screenshot.getWidth();
        h = screenshot.getHeight();
        s = screenshot.getStride();
        f = screenshot.getFormat();
        fprintf(stdout, "ffffffffffff %d \r\n", f);
        size = screenshot.getSize();
    }
     */
    if (mapbase != MAP_FAILED) {
        munmap((void *)mapbase, mapsize);
    }
    return 0;
}
