
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "WQApplication.hpp"
#include "DispatchThread.hpp"
#include <QDebug>

namespace Wt {

DispatchObject::DispatchObject(DispatchThread *thread)
  : _thread(thread)
{
  connect(this, SIGNAL(doEvent()), this, SLOT(onEvent()));
}

void DispatchObject::propagateEvent()
{
  emit doEvent();
}

void DispatchObject::onEvent()
{
  _thread->doEvent();
}

DispatchThread::DispatchThread(WQApplication *app,
bool withEventLoop)
  : QThread(),
    _app(app),
    _qtEventLoop(withEventLoop),
    _dispatchObject(0),
    _event(0),
    _done(false),
    _newEvent(false)
{ }

void DispatchThread::run()
{
    // app_->createQt();
    // qDebug() << "--- DispatchThread run: Thr=" << QThread::currentThread();

    if (_qtEventLoop)
        _dispatchObject = new DispatchObject(this);

    signalDone();

    if (_qtEventLoop)
        exec();
    else
        myExec();

    delete _dispatchObject;

    // app_->destroyQt();

    signalDone();
}

void DispatchThread::myExec()
{
  boost::mutex::scoped_lock lock(_newEventMutex);

  for (;;) {
    if (!_newEvent)
      _newEventCondition.wait(lock);

    doEvent();

    if (_app->_finalized)
      return;

    _newEvent = false;
  }
}

void DispatchThread::myPropagateEvent()
{
  boost::mutex::scoped_lock lock(_newEventMutex);
  _newEvent = true;
  _newEventCondition.notify_one();
}

void DispatchThread::signalDone()
{
  boost::mutex::scoped_lock lock(_doneMutex);
  _done = true;
  _doneCondition.notify_one();
}

void DispatchThread::waitDone()
{
  boost::mutex::scoped_lock lock(_doneMutex);

  if (_done)
    return;
  else
    _doneCondition.wait(lock);
 }

void DispatchThread::notify(const WEvent& event)
{
  _event = &event;

  _done = false;

  if (_dispatchObject)
    _dispatchObject->propagateEvent();
  else
    myPropagateEvent();

  waitDone();
}

void DispatchThread::destroy()
{
  if (_qtEventLoop)
    QThread::exit();
}

void DispatchThread::doEvent()
{
    // qDebug() << "--- DispatchThread doEvent: Thr=" << QThread::currentThread();

    _app->attachThread(true);

    _app->realNotify(*_event);
    signalDone();

    _app->attachThread(false);
}

}
