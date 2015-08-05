var a;
a = 2;

var st = DHAS.initiateAction("/rest/js/test");
DHAS.log(st);

DHAS.onevent = function(json)
{
    var obj = JSON.parse(json);
    DHAS.log(obj.event);
};
/*
syntax error

function onEvent(json)
{
    var obj = JSON.parse(json);
    log("got event: "+obj.event);
};*/
