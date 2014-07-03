/*
 * console_std.cpp
 *
 *  Created on: Jul 13, 2014
 *      Author: lion
 */

#include "console.h"

namespace fibjs
{

static int32_t s_loglevel = console_base::_NOTSET;

std_logger s_std;

#define MAX_LOGGER  10
static obj_ptr<logger> s_logs[MAX_LOGGER];

void asyncLog(int priority, std::string msg)
{
    if (priority <= s_loglevel)
    {
        int32_t i;

        for (i = 0; i < MAX_LOGGER && s_logs[i]; i ++)
            s_logs[i]->log(priority, msg);

        if (i == 0)
            s_std.log(priority, msg);
    }
}

void flushLog()
{
    int32_t i;

    for (i = 0; i < MAX_LOGGER && s_logs[i]; i ++)
        s_logs[i]->flush();

    s_std.flush();
}

result_t console_base::get_loglevel(int32_t &retVal)
{
    retVal = s_loglevel;
    return 0;
}

result_t console_base::set_loglevel(int32_t newVal)
{
    s_loglevel = newVal;
    return 0;
}

result_t console_base::get_colors(obj_ptr<TextColor_base> &retVal)
{
    retVal = logger::get_std_color();
    return 0;
}

result_t console_base::config(v8::Local<v8::Array> cfg)
{
    reset();

    int32_t sz = cfg->Length();
    int32_t i;

    for (i = 0; i < sz; i ++)
    {
        v8::Local<v8::Value> v = cfg->Get(i);
        v8::Local<v8::Value> type;
        v8::Local<v8::Object> o;

        if (v->IsString() || v->IsStringObject())
        {
            type = v;
            o = v8::Object::New(isolate);
        }
        else if (v->IsObject())
        {
            o = v8::Local<v8::Object>::Cast(v);
            type = o->Get( v8::String::NewFromUtf8(isolate, "type",
                                                   v8::String::kNormalString, 4));

            if (IsEmpty(type))
                return CALL_E_INVALIDARG;
        }
        else
            return CALL_E_INVALIDARG;

        v8::String::Utf8Value s(type);
        if (!*s)
            return CALL_E_INVALIDARG;

        if (!qstrcmp(*s, "console"))
            s_logs[i] = new std_logger(o);
        else if (!qstrcmp(*s, "syslog"))
        {
#ifndef _WIN32
            s_logs[i] = new sys_logger(o);
#endif
        }
    }

    return 0;
}

result_t console_base::reset()
{
    int32_t i;

    for (i = 0; i < MAX_LOGGER; i ++)
        if (s_logs[i])
        {
            s_logs[i]->stop();
            s_logs[i].Release();
        }

    return 0;
}

}