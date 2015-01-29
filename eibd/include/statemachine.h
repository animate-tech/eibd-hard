/*----------------------------------------------------------------------------*/
/* Copyright (c) 2008 John Downey (jtdowney@purdue.edu)                       */
/*
/*                                                                            */
/* Permission is hereby granted, free of charge, to any person obtaining a    */
/* copy of this software and associated documentation files (the "Software"), */
/* to deal in the Software without restriction, including without limitation  */
/* the rights to use, copy, modify, merge, publish, distribute, sublicense,   */
/* and/or sell copies of the Software, and to permit persons to whom the      */
/* Software is furnished to do so, subject to the following conditions:       */
/*                                                                            */
/* The above copyright notice and this permission notice shall be included in */
/* all copies or substantial portions of the Software.                        */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    */
/* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        */
/* DEALINGS IN THE SOFTWARE.                                                  */
/*----------------------------------------------------------------------------*/

/*
 *   massive rewrite to support many features & stabilize
 *
 *    Embedded Controller software
 *    Copyright (c) 2006- Z2, GmbH, Switzerland
 *    All Rights Reserved
 *
 *    THE ACCOMPANYING PROGRAM IS PROPRIETARY SOFTWARE OF Z2, GmbH,
 *    AND CANNOT BE DISTRIBUTED, COPIED OR MODIFIED WITHOUT
 *    EXPRESS PERMISSION OF Z2, GmbH.
 *
 *    Z2, GmbH DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 *    SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 *    AND FITNESS, IN NO EVENT SHALL Z2, LLC BE LIABLE FOR ANY
 *    SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 *    IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 *    ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 *    THIS SOFTWARE.
 *
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <map>
#include <queue>

template<class S, class E, class T>
class StateMachine : LoggableObjectInterface
  {
  private:
    typedef void (T::*Callback)(void);

    T *m_instance;
    std::map<std::pair<S, E>, Callback> m_transitions;
    S m_currentState;

    std::queue<E> eventqueue;
    bool debug;
    Logs *tr;
    std::string name;
  public:
    StateMachine(T *instance, bool debug=false, Logs *tr=NULL, const std::string &name="FSM") :
        m_instance(instance),
        debug(debug),
        tr(tr),
        name(name)
    {
    }

    ~StateMachine(void)
    {
      while (!eventqueue.empty())
        {
          eventqueue.pop();
        }
    }

    void
    SetStateCallback(S state, E event, Callback callback)
    {
      std::pair<S, E> k(state, event);
      m_transitions[k] = callback;
    }

    S
    GetCurrentState(void) const
    {
      return m_currentState;
    }

    void
    SetCurrentState(S state)
    {
      if (debug && tr) {
          TRACEPRINTF(tr, 4, this,
                            "state set from %d to %d", (int) m_currentState, (int) state);
      }
      m_currentState = state;
    }

    void
    PushEvent(E e)
    {
      if (debug && tr) {
          TRACEPRINTF(tr, 4, this,
                            "new event %d", (int) e);
      }

      eventqueue.push(e);
    }

    bool
    EventsPending() const
    {
      return !eventqueue.empty();
    }

    Callback GetTransition(S s, E e)
    {
      std::pair<S, E> k(s, e);
      return m_transitions[k];
    }

    void
    ConsumeEvent()
    {
      S ostate = m_currentState;
      E e = eventqueue.front();
      std::pair<S, E> k(m_currentState, e);
      eventqueue.pop();

      if (debug && tr)
        {
          TRACEPRINTF(tr, 4, this, "from state %d input event %d ",
              (int) ostate, (int) e);
        }
      Callback callback = m_transitions[k];
      // assure there is a transition
      if (!callback) {
          ERRORLOG(tr, LOG_EMERG, this,
              "no state transition defined for state %d receiving event %d, FSM goes down ",
              (int) m_currentState,
              (int) e);
          assert(callback);
      }

      (m_instance->*callback)();
      if (debug && tr)
        {
          TRACEPRINTF(tr, 4, this, "transition state %d, event %d -> state %d",
              (int) ostate, (int) e, (int) m_currentState);
        }

    }

    const char *_str(void) const
    {
      return name.c_str();
    }
};

#endif
