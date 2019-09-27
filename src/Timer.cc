// See the file "COPYING" in the main distribution directory for copyright.

#include "zeek-config.h"

#include <uv.h>

#include "util.h"
#include "Timer.h"
#include "Desc.h"
#include "broker/Manager.h"
#include "iosource/Manager.h"

// Names of timers in same order than in TimerType.
const char* TimerNames[] = {
	"BackdoorTimer",
	"BreakpointTimer",
	"ConnectionDeleteTimer",
	"ConnectionExpireTimer",
	"ConnectionInactivityTimer",
	"ConnectionStatusUpdateTimer",
	"DNSExpireTimer",
	"FileAnalysisInactivityTimer",
	"FlowWeirdTimer",
	"FragTimer",
	"InterconnTimer",
	"IPTunnelInactivityTimer",
	"NetbiosExpireTimer",
	"NetWeirdTimer",
	"NetworkTimer",
	"NTPExpireTimer",
	"ProfileTimer",
	"RotateTimer",
	"RemoveConnection",
	"RPCExpireTimer",
	"ScheduleTimer",
	"TableValTimer",
	"TCPConnectionAttemptTimer",
	"TCPConnectionDeleteTimer",
	"TCPConnectionExpireTimer",
	"TCPConnectionPartialClose",
	"TCPConnectionResetTimer",
	"TriggerTimer",
	"TimerMgrExpireTimer",
};

const char* timer_type_to_string(TimerType type)
	{
	return TimerNames[type];
	}

void Timer::Describe(ODesc* d) const
	{
	d->Add(TimerNames[type]);
	d->Add(" at " );
	d->Add(Time());
	}

unsigned int TimerMgr::current_timers[NUM_TIMER_TYPES];

TimerMgr::~TimerMgr()
	{
	DBG_LOG(DBG_TM, "deleting timer mgr %p", this);
	}

int TimerMgr::Advance(double arg_t, int max_expire)
	{
	DBG_LOG(DBG_TM, "advancing %stimer mgr %p to %.6f",
		this == timer_mgr ? "global " : "", this, arg_t);

	t = arg_t;
	last_timestamp = 0;
	num_expired = 0;
	last_advance = timer_mgr->Time();
	broker_mgr->AdvanceTime(arg_t);

	return DoAdvance(t, max_expire);
	}


struct TimerData {
	Timer* timer;
	UV_TimerMgr* mgr;
};

static void timer_callback(uv_timer_t* handle)
	{
	uv_handle_t* h = reinterpret_cast<uv_handle_t*>(handle);
	if ( auto data = reinterpret_cast<TimerData*>(uv_handle_get_data(h)) )
		data->mgr->Dispatch(data->timer);
	}

static void close_callback(uv_handle_t* handle)
	{
	delete handle;
	}

void UV_TimerMgr::Add(Timer* timer)
	{
	uv_timer_t *handle = new uv_timer_t();

	int r = uv_timer_init(iosource_mgr->GetLoop(), handle);
	if ( r != 0 )
		{
		DBG_LOG(DBG_TM, "Timer failed to init: %s", uv_strerror(r));
		delete handle;
		return;
		}

	TimerData* data = new TimerData();
	data->timer = timer;
	data->mgr = this;
	
	uv_handle_set_data(reinterpret_cast<uv_handle_t*>(handle), data);
	auto it = timers.emplace(timer, handle);

	r = uv_timer_start(handle, timer_callback, static_cast<uint64_t>(timer->Time()) * 1000, 0);
	if ( r != 0 )
		{
		DBG_LOG(DBG_TM, "Timer failed to start: %s", uv_strerror(r));
		timers.erase(it.first);
		delete handle;
		return;
		}

	cumulative++;
	if ( timers.size() > peak_size )
		peak_size = (int)timers.size();
	}

void UV_TimerMgr::Expire()
	{
	while ( ! timers.empty() )
		{
		auto entry = timers.begin();
		Dispatch(entry, true);
		}
	}

void UV_TimerMgr::Dispatch(Timer* timer)
	{
	auto it = timers.find(timer);
	if ( it == timers.end() )
		return;

	Dispatch(it, false);
	}

void UV_TimerMgr::Dispatch(TimerMap::const_iterator entry, bool is_expire)
	{
	Timer* timer = entry->first;
	uv_timer_t* handle = entry->second;
	timers.erase(entry);
	
	timer->Dispatch(network_time, is_expire);
	delete timer;
	
	uv_timer_stop(handle);
	uv_close(reinterpret_cast<uv_handle_t*>(handle), close_callback);
	}

void UV_TimerMgr::Remove(Timer* timer)
	{
	auto it = timers.find(timer);
	if ( it == timers.end() )
		return;

	uv_timer_t* handle = it->second;
	timers.erase(it);

	if ( uv_is_closing(reinterpret_cast<uv_handle_t*>(handle)) != 0 )
		{
		uv_timer_stop(handle);
		uv_close(reinterpret_cast<uv_handle_t*>(handle), close_callback);
		}

	delete timer;
	}
