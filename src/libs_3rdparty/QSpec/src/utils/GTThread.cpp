/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
 * http://ugene.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <core/MainThreadRunnable.h>

#include <QCoreApplication>
#include <QThread>

#include "GTGlobals.h"
#include "core/MainThreadTimer.h"
#include "utils/GTThread.h"

namespace HI {

#define GT_CLASS_NAME "ThreadWaiter"
const qint64 TIMER_INTERVAL = 100;

ThreadWaiter::ThreadWaiter()
    :  // os(os),
      startValue(0),
      endValue(0) {
}

#define GT_METHOD_NAME "wait"
void ThreadWaiter::wait() {
    MainThreadTimer mainThreadTimer(TIMER_INTERVAL);
    startValue = mainThreadTimer.getCounter();
    while (endValue <= startValue) {
        GTGlobals::sleep(TIMER_INTERVAL);
        endValue = mainThreadTimer.getCounter();
    }
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTThread"

#define GT_METHOD_NAME "waitForMainThread"
void GTThread::waitForMainThread() {
    ThreadWaiter waiter;
    waiter.wait();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "runInMainThread"
void GTThread::runInMainThread(GUITestOpStatus &os, CustomScenario *scenario) {
    qDebug("Running in the main thread");
    MainThreadRunnable::runInMainThread(os, scenario);
    qDebug("Run in the main thread is finished");
}
#undef GT_METHOD_NAME

bool GTThread::isMainThread() {
    QThread *currentThread = QThread::currentThread();
    QThread *mainThread = QCoreApplication::instance()->thread();
    return currentThread == mainThread;
}

#undef GT_CLASS_NAME

}  // namespace HI
