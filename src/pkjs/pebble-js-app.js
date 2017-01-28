var initialized = false;

Pebble.addEventListener("ready", function() {
    console.log("ready function has been called");
    initialized = true;
});

function getValue(name, defaultValue) {
    var value = localStorage.getItem(name);
    if (value === null || value === "undefined") {
        value = defaultValue;
        localStorage.setItem(name, value);
    }
    return value;
}

Pebble.addEventListener("showConfiguration", function() {
    console.log("showing configuration");
    console.log("    day0 = " + localStorage.getItem("day0"));
//    console.log("    day1 = " + localStorage.getItem("day1"));
//    console.log("    day2 = " + localStorage.getItem("day2"));
//    console.log("    day3 = " + localStorage.getItem("day3"));
//    console.log("    day4 = " + localStorage.getItem("day4"));
//    console.log("    lessons = " + localStorage.getItem("lessons"));
//    console.log("    vibration = " + localStorage.getItem("vibration"));
//    console.log("    breaks = " + localStorage.getItem("breaks"));
//    console.log("    table = " + localStorage.getItem("table"));
    //console.log("    day0 = " + getValue("day0", "0655N15N10N20N10N10N10N10N10N10N10N10N10"));
    console.log("    day0 = " + getValue("day0", "0740N5N5N10N20N10N10N5N10N5N10N5N5"));
    console.log("    day1 = " + getValue("day1", ""));
    console.log("    day2 = " + getValue("day2", ""));
    console.log("    day3 = " + getValue("day3", ""));
    console.log("    day4 = " + getValue("day4", ""));
    console.log("    lessons = " + getValue("lessons", "45"));
    console.log("    vibration = " + getValue("vibration", "L:s0e0;S:s1e-1"));
    console.log("    breaks = " + getValue("breaks", "0b,Bb,Sb"));
    //console.log("    table = " + getValue("table", "MO:0s1f1f1f1f1f1,TU:0f1f1f1s1f1s,WE:0f0f0f1s1f1f1,TH:0f1f1s0s1f1f0f0f1,FR:1f1f1f1f1"));
    console.log("    table = " + getValue("table", "MO:1s1f1f1f1f1f1,TU:1s1f1f1f1f1f1f1f1f1,WE:1s1f1f1f1f1f1,TH:1s1f1f1f1f1f1f1f1,FR:1s1f1f1f1f1f1"));
    console.log("go on");
    Pebble.openURL('https://mkocer.github.io/TeachersWatch/html/TeachersWatch.html'  + '?day0=' + encodeURIComponent(localStorage.getItem("day0"))   + '&day1=' + encodeURIComponent(localStorage.getItem("day1"))  + '&day2=' + encodeURIComponent(localStorage.getItem("day2")) + '&day3=' + encodeURIComponent(localStorage.getItem("day3")) + '&day4=' + encodeURIComponent(localStorage.getItem("day4")) + '&lessons=' + encodeURIComponent(localStorage.getItem("lessons")) + '&vibration=' + encodeURIComponent(localStorage.getItem("vibration")) + '&breaks=' + encodeURIComponent(localStorage.getItem("breaks")) + '&table=' + encodeURIComponent(localStorage.getItem("table")) );
});

Pebble.addEventListener("webviewclosed", function(e) {
    console.log("configuration closed");
    // webview closed
    if ((typeof (e.response) == 'string') && (e.response.length > 0)) {
        try {
            //var options = JSON.parse(decodeURIComponent(e.response));
            var options = JSON.parse(e.response);
            console.log("Options = " + JSON.stringify(options));
            localStorage.setItem("day0", decodeURIComponent(options.day0));
            localStorage.setItem("day1", decodeURIComponent(options.day1));
            localStorage.setItem("day2", decodeURIComponent(options.day2));
            localStorage.setItem("day3", decodeURIComponent(options.day3));
            localStorage.setItem("day4", decodeURIComponent(options.day4));
            localStorage.setItem("lessons", decodeURIComponent(options.lessons));
            localStorage.setItem("vibration", decodeURIComponent(options.vibration));
            localStorage.setItem("breaks", decodeURIComponent(options.breaks));
            localStorage.setItem("table", decodeURIComponent(options.table));
            console.log("send options");
            Pebble.sendAppMessage(options);
        } catch (e) {
        }
    }
});
