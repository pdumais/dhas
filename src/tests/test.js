var a;
a = 2;

var st = initiateAction("/rest/js/test");
log(st);

syntax error

function onEvent(json)
{
    var obj = JSON.parse(json);
    log("got event: "+obj.event);
};
