initiateAction("/insteon/setcontroller?id=0x112233")
initiateAction("/insteon/clearmodules")
initiateAction("/insteon/addmodule?name=Outside&id=0x111111")
initiateAction("/insteon/addmodule?name=bedroom&id=0x444444")
initiateAction("/insteon/addezflora?name=EZFlora&id=0x222222")
initiateAction("/insteon/ezflora/setprogram?id=0x222222&p=1&z1=45&z2=45&z3=45")
initiateAction("/weather/setip?ip=192.168.5.5")
initiateAction("/phone/register?user=someuser&pin=somepassword&proxy=192.168.5.250:5060")
initiateAction("/phone/blf?ext=mainline")
initiateAction("/io/addwebrelay?ip=192.168.5.7&id=1&name=r1")
initiateAction("/io/addwebrelay?ip=192.168.5.7&id=2&name=r2")


function onTimer(ts)
	temp = os.date("*t",ts)
    -- at 23h59, turn off outside light
	if temp["hour"]==23 and temp["min"]==59 then
		initiateAction("/insteon/switch?id=111111&action=off")
	end
end

function onScheduledEvent(v)
    if v["name"]=="wakeup" then
        local ext = v["param"]
        local request = "/phone/call?ext="..ext.."&play=wakeup&releaseaftersounds=true"
        initiateAction(request)
        if v["param"]=="720" then
            -- if bedroom phone, then turn on light in bedroom
            initiateAction("/insteon/switch?id=0x444444&action=on&level=50")
        end
    end
end

function onEvent(st)
    JSON = (loadfile "/opt/ha/JSON.lua")()
    local v = JSON:decode(st)

   	if v["event"]=="timer" then onTimer(v["timestamp"]) end

	if v["event"]=="scheduledevent" then
        -- process sceduled events separately
        onScheduledEvent(v)
	end
end
