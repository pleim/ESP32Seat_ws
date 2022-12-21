var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage; // <-- add this line
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    uiupdate(event.data)
}

function onLoad(event) {
    initWebSocket();
    document.getElementById('btn_save').addEventListener('click', save);

    // Mode
    document.getElementById('mode_auto').addEventListener('change', send);
    document.getElementById('mode_axis').addEventListener('change', send);

    // Parameters
    document.getElementById('par_speed').addEventListener('change', send);
    document.getElementById('par_top').addEventListener('change', send);
    document.getElementById('par_bottom').addEventListener('change', send);
    document.getElementById('par_currmax').addEventListener('change', send);
    document.getElementById('par_hyst').addEventListener('change', send);
    document.getElementById('par_offset').addEventListener('change', send);
    document.getElementById('par_sens').addEventListener('change', send);
}

function uiupdate(data) {
    console.log("values", data);
    var values = data.split(";");
    
    if (values[0] == 'state') {
        if (values.length < 10) return;        
        document.getElementById('state_pitch').innerHTML = values[1];
        document.getElementById('state_pos').innerHTML = values[2];
        document.getElementById('state_speed').innerHTML = values[3];
        document.getElementById('state_current').innerHTML = values[4];
        document.getElementById('state_voltage').innerHTML = values[5];
        document.getElementById('mode_auto').checked = (values[6] == '1');
        document.getElementById('mode_axis').checked = (values[7] == '1');
        var c_up = 'lightgray';
        var c_down = 'lightgray';
        if(values[8] == "1") c_up = 'green'; 
        if(values[9] == "1") c_down = 'green';
        document.getElementById('move_up').style.color = c_up;
        document.getElementById('move_down').style.color = c_down;   
    }

    if (values[0] == 'par') {
        if (values.length != 8) return;
        document.getElementById('par_speed').value = values[1];
        document.getElementById('par_top').value = values[2];
        document.getElementById('par_bottom').value = values[3];
        document.getElementById('par_currmax').value = values[4];
        document.getElementById('par_hyst').value = values[5];
        document.getElementById('par_offset').value = values[6];
        document.getElementById('par_sens').value = values[7];
    }
}

function send(event) {    
    var msg;
    switch (event.target.type) {
        case 'checkbox':
            msg = event.target.id + '=' + event.target.checked;
            break;

        case 'number':
            msg = event.target.id + '=' + event.target.value;
            break;

        default:
            msg = 'n.d.'
            break;
    }

    console.log(msg);
    websocket.send(msg);
}

function test(e) {
    uiupdate(e.target.value);
}

function save(e)
{
    console.log('save');
    websocket.send('save');    
}
