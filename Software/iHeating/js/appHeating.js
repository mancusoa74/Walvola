//========== globals =========
var appHeating; //main application
var mqtt; //mqtt client
var $$; //F7 DOM accessor

var querying_walvola_status = false;

//========= constants =========
const MQTT_UNAME = "YOUR MQTT BROKER ACCOUNT USER NAME";
const MQTT_PASSW = "YOUR MQTT BROKER ACCOUNT PASSWORD";
const PUBNUB_BROADCAST_WALVOLA_DST = "allwalvoles"; //broadcast walvla address. 
const MQTT_WALVOLAS_TOPIC = "YOUR TOPIC/in/";
const MQTT_CONTROLLERS_TOPIC = "YOUR TOPIC/out/+";

const APPID = Math.floor(Math.random()*1000).toString();//"app1234";
const CMD_GET_STATUS = "GET_STATUS";
const CMD_SET_STATUS = "SET_STATUS";
const TYPE_REQUEST = "REQUEST";
const TYPE_REPLY = "REPLY";

//========= objects =========
var mqtt_message = function (data) {
    return { 
            src: data.src,
            dst: data.dst,
            type: TYPE_REQUEST,
            cmd: data.cmd,
            value: data.value
    }
}

//========= main =========
init_F7(); //initialize F7 application
appHeating.init(); //start F7 application
init_mqtt(); //initialize pubnub communication
//========= support functions =========

function init_F7() {    
    console.log("inizializing F7 app");
    // Initialize Framework7 app
    appHeating = new Framework7({
        init: false //Disable App's automatica initialization
    });
    $$ = Dom7;

    //Add main view
    var mainView = appHeating.addView('.view-main', {
        dynamicNavbar: true
    });

    appHeating.onPageInit('index', function (page) {  
        console.log("page init");
        empty_walvole_list();
        console.log("Getting status of Walvola(s)");
        //app_notify("Getting status of Walvola(s)", 5000);
    });
    console.log("F7 app initialized");
}

// function query_walvole_status() {
//     appHeating.showPreloader('Interrogazione Walvole');
//     querying_walvola_status = true;
//     mqtt_send_mex(PUBNUB_BROADCAST_WALVOLA_DST, /*PUBNUB_BROADCAST_TOPIC,*/ CMD_GET_STATUS, '0');
//     var query_interval = setInterval(function () {
//         console.log("CMD_GET_STATUS timer tick");
//         if (querying_walvola_status == true) {
//             console.log("re-sending CMD_GET_STATUS");
//             querying_walvola_status = true;
//             mqtt_send_mex(PUBNUB_BROADCAST_WALVOLA_DST, /*PUBNUB_BROADCAST_TOPIC,*/ CMD_GET_STATUS, '0');
//         } else {
//             console.log("stopping CMD_GET_STATUS timer");
//             clearInterval(query_interval);
//         }
//     }, 1113000);
// }

function empty_walvole_list() {
    console.log("emptying list");
    $$('#list-walvole').html('');
    console.log("list empty");
}

function init_mqtt() {
    console.log("initializing MQTT");

    mqtt = new Paho.MQTT.Client("m20.cloudmqtt.com", 37780, APPID);
    
    mqtt.onConnectionLost = function (responseObject) {
        console.log("Connection Lost: "+responseObject.errorMessage);
    }
    
    mqtt.onMessageArrived = function (message) {
        console.log("Message Arrived: "+message.destinationName);
        console.log(typeof(message));
        console.log("Message Arrived: "+message.payloadString);
        console.log("CALLING MQTT_PROCESS_MEX");
        mqtt_process_mex(message.payloadString);
    }

    function onConnect() {
        console.log("onConnect");
        //mqtt.subscribe(MQTT_CONTROLLERS_TOPIC, {qos: 1});
        mqtt.subscribe(MQTT_CONTROLLERS_TOPIC);
        console.log("MQTT subscribed to " + MQTT_CONTROLLERS_TOPIC);
        console.log("mqtt initialized");
        //query_walvole_status();
    }

    function onFailure(invocationContext, errorCode, errorMessage) {
        console.log("Error = " + errorMessage + "[" + errorCode + "]");
    }

    mqtt.connect({
        useSSL: true,
        userName: MQTT_UNAME,
        password: MQTT_PASSW,
        onSuccess: onConnect,
        onFailure: onFailure,
    });
}

// function pubnub_send_mex(dst, topic, command, value) {
function mqtt_send_mex(dst, command, value) {
    console.log("sending mqtt message")
    mqtt_mex_obj = mqtt_message({src: APPID, dst: dst, cmd: command, value: value});
    console.log(mqtt_mex_obj);
    mqtt_mex_json =JSON.stringify(mqtt_mex_obj); 
    console.log(mqtt_mex_json);

    message = new Paho.MQTT.Message(mqtt_mex_json);
    message.destinationName = MQTT_WALVOLAS_TOPIC + dst;
    message.retained = true;
    mqtt.send(message);

    console.log("mqtt message sent");
}

function mqtt_process_mex(mex){
    console.log(mex);
    console.log(typeof mex);
    console.log(mex.length);
    
    var mqtt_mex = JSON.parse(mex); 
    
    console.log(mqtt_mex);
    console.log(mqtt_mex.time);
    console.log(mqtt_mex.src);
    console.log(mqtt_mex.dst);
    console.log(mqtt_mex.label);
    console.log(mqtt_mex.type);
    console.log(mqtt_mex.cmd);
    console.log(mqtt_mex.value);

    
    if (typeof mqtt_mex.time !== 'undefined') {
        console.log("---------------------");
        console.log("time is VALID");
        current_time = Date.now();
        mex_time = new Date(mqtt_mex.time.substring(0,4),mqtt_mex.time.substring(5,7) - 1, mqtt_mex.time.substring(8,10), mqtt_mex.time.substring(12,14), mqtt_mex.time.substring(15,17), mqtt_mex.time.substring(18,20), 0).getTime();
        console.log(current_time);
        console.log(mex_time);
        console.log("---------------------");
        console.log(current_time - mex_time);
        

        if ((current_time - mex_time) < 700000) {
            console.log("messagio aggiornato");
            if (mqtt_mex.src == APPID)
                console.log("discarding mqtt mesaage as sent by myself");
            else {
                console.log("processing mqtt message");
                //{"src":"valvola_src", label" : "SALONE", "type": "REPLY", "cmd":"STATUS","value": {"voltage":"2.86", "state":"OFF"}}
                if (mqtt_mex.type == TYPE_REPLY && mqtt_mex.cmd == CMD_GET_STATUS) {
                    console.log("received STATUS replay message from walvola: " + mqtt_mex.src + " value = " + mqtt_mex.value.voltage);
                    add_update_walvola(mqtt_mex);
                    add_hook(mqtt_mex);
                    querying_walvola_status = false;
                    appHeating.hidePreloader();
                    console.log("mqtt message processed");
                }
                if (mqtt_mex.type == TYPE_REPLY && mqtt_mex.cmd == CMD_SET_STATUS) {
                    console.log("received STATUS replay message from walvola: " + mqtt_mex.src + " value = " + mqtt_mex.value);
                    add_update_walvola(mqtt_mex);
                    // location.reload();
                    //appHeating.hidePreloader();
                    console.log("mqtt message processed");
                }
            }
        }
        else { console.log("Discarding message as too old");}
    }
}

// function app_notify(mex, timeout) {
//     appHeating.addNotification({
//         title: mex,
//         hold: timeout,
//     });
// }
function add_update_walvola(mqtt_mex) { 
    console.log("ADD_UPDATED_WALVOLA");
    console.log(mqtt_mex);
    var bg_state_class;
    var html = `<li class="swipeout" id="#WALVOLA_ID#-swipeout">
                    <div class="swipeout-content item-content">
                      <div class="item-inner">
                        <div class="item-title" id="t1">#WALVOLA_LABEL#</div>
                        <div class="item-after"><span class="badge">#WALVOLA_VOLT#</span></div>
                        <div class="item-after"><span id="#WALVOLA_ID#-badge" class="badge #WALVOLA_STATE_CLASS#">#WALVOLA_STATE#</span></div>
                      </div>
                    </div>
                    <div class="swipeout-actions-right">
                      <a href="#" class="bg-green #WALVOLA_ID#_ACTON_ON">ON</a>
                      <a href="#" class="bg-red #WALVOLA_ID#_ACTON_OFF">OFF</a>
                    </div>
                  </li>`;
    //console.log(html);
    html = html.replace(/#WALVOLA_LABEL#/g, mqtt_mex.label);
    html = html.replace(/#WALVOLA_ID#/g, mqtt_mex.src);
    html = html.replace(/#WALVOLA_VOLT#/g, mqtt_mex.value.voltage);
    html = html.replace(/#WALVOLA_STATE#/g, mqtt_mex.value.state);
    if (mqtt_mex.value.state == "ON")
        bg_state_class = "bg-green"
    else if (mqtt_mex.value.state == "OFF")
        bg_state_class = "bg-red"
    html = html.replace("#WALVOLA_STATE_CLASS#", bg_state_class);

    if ($$('#' + mqtt_mex.src + '-swipeout').length == 0) {
        console.log("walvola not existing.");
        $$('#list-walvole').append(html);
    }
    else if ($$('#' + mqtt_mex.src + '-swipeout').length == 1) {
        console.log("walvola already existing");
        $$('#' + mqtt_mex.src + '-swipeout').html(html);
    }

    //console.log(html);
}

function add_hook(mqtt_mex) {
    console.log("adding hook for walvolva");
    $$('#list-walvole').on('click', 'a.' + mqtt_mex.src + '_ACTON_ON.bg-green', function() {walvola_set_state(mqtt_mex.src, "ON");});
    $$('#list-walvole').on('click', 'a.' + mqtt_mex.src + '_ACTON_OFF.bg-red', function() {walvola_set_state(mqtt_mex.src, "OFF");});
    console.log("hook added");
} 

function walvola_set_state(walvola, state) {
    console.log("setting state= " + state + " of walvola= " + walvola);
    //appHeating.showPreloader('Impostazione Walvola a ' + state);
    mqtt_send_mex(walvola,/*PUBNUB_BROADCAST_TOPIC,*/ CMD_SET_STATUS, state);
    $$('#' + walvola +'-badge').html("TRANSITION");
    if (state == "ON") {
        $$('#' + walvola +'-badge').addClass("bg-gray").removeClass("bg-red");
    }
    else {
        $$('#' + walvola +'-badge').addClass("bg-gray").removeClass("bg-green");   
    }

    console.log("walvola state set");
}
