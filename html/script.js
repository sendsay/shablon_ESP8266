var xmlHttp = new XMLHttpRequest();

function getData() {

    if(xmlHttp.readyState==0 || xmlHttp.readyState==4){
        xmlHttp.open('GET','/getData',true);
        xmlHttp.send();
        xmlHttp.onload = function(e) {

            var str = xmlHttp.responseText;

            json=JSON.parse(str);

            //wifi
            document.getElementById("ssid").value = json.ssid;
            document.getElementById("password").value = json.password;
            document.getElementById("ssidAP").value = json.ssidAP;
            document.getElementById("passwordAP").value = json.passwordAP;
            //time
            document.getElementById("timezone").value = json.timezone;
            if (json.summertime == 1) {document.getElementById("summerTime").checked = true; }
            document.getElementById("ntpServerName").value = json.ntpServerName;

            console.log(str);
        }
    }
}

function val(id){
var v = document.getElementById(id).value;
return v;
}

function val_sw(nameSwitch) {
var switchOn = document.getElementById(nameSwitch);
if (switchOn.checked){
    return 1;
}
return 0;
}

function restartButton() {
    xmlHttp.open("GET", "/restart", true);
    xmlHttp.send();
}

function saveButton() {
    var content = "/saveContent?ssid=" + val('ssid') + "&password=" + val('password') + "&ssidAP=" + val('ssidAP') +
                    "&passwordAP=" + val('passwordAP') + "&timezone=" + val('timezone') + "&summertime=" + val_sw('summerTime') +
                    "&sigOn=" +"&ntpServerName=" + val('ntpServerName');

    console.log("************* send to server ");
    console.log(content);

    xmlHttp.open('GET', content,true);
    xmlHttp.send();
}
