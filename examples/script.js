initiateAction("/insteon/setcontroller?id=0x112233");
initiateAction("/insteon/clearmodules");
initiateAction("/insteon/addmodule?name=Outside&id=0x111111");
initiateAction("/insteon/addmodule?name=bedroom&id=0x444444");
initiateAction("/insteon/addezflora?name=EZFlora&id=0x222222");
initiateAction("/insteon/ezflora/setprogram?id=0x222222&p=1&z1=45&z2=45&z3=45");
initiateAction("/weather/setip?ip=192.168.5.5");
initiateAction("/phone/register?user=someuser&pin=somepassword&proxy=192.168.5.250:5060");
initiateAction("/phone/blf?ext=mainline");
initiateAction("/io/addwebrelay?ip=192.168.5.7&id=1&name=r1");
initiateAction("/io/addwebrelay?ip=192.168.5.7&id=2&name=r2");


function onTimer(ts)
{
    var temp = new Date(ts*1000);
    var h = temp.getHours();
    var m = temp.getMinutes();

    // at 23h59, turn off outside light
	if (h==23 && m==59)
    {
		initiateAction("/insteon/switch?id=111111&action=off");
	}
}

function onScheduledEvent(v)
{
    if (v["name"]=="wakeup")
    {
        var ext = v["param"];
        var request = "/phone/call?ext=" + ext + "&play=wakeup&releaseaftersounds=true";
        initiateAction(request);
        if (v["param"]=="720")
        {
            // if bedroom phone, then turn on light in bedroom
            initiateAction("/insteon/switch?id=0x444444&action=on&level=50");
        }
    }
}

function onEvent(st)
{
    var v = JSON.parse(st);

   	if (v["event"]=="timer") onTimer(v["timestamp"]);

	if (v["event"]=="scheduledevent")
    {
        // process sceduled events separately
        onScheduledEvent(v);
	}
}
