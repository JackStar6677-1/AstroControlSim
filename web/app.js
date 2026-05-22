const grid = document.getElementById('antenna-grid');
const connStatus = document.getElementById('connection-status');
const lastUpdateText = document.getElementById('last-update');

const stats = {
    active: document.getElementById('active-count'),
    error: document.getElementById('avg-error'),
    flux: document.getElementById('total-flux')
};

const STATE_COLORS = {
    0: 'status-idle',      // IDLE (Gray)
    1: 'status-slewing',   // SLEWING (Yellow)
    2: 'status-tracking',  // TRACKING (Green)
    3: 'status-fault'      // FAULT (Red)
};

// Initialize the 50 cards
const antennaCards = {};

for (let i = 1; i <= 50; i++) {
    const card = document.createElement('div');
    card.className = 'glass p-3 rounded-xl antenna-card flex flex-col items-center justify-center relative group';
    card.innerHTML = `
        <span class="text-[8px] text-gray-500 font-bold">ANT-${i.toString().padStart(2, '0')}</span>
        <div class="antenna-dot w-3 h-3 rounded-full bg-gray-600 my-1 transition-colors duration-500"></div>
        <span class="text-[9px] font-mono state-label text-gray-600 uppercase">OFFLINE</span>
        
        <!-- Hover Sensor Overlay -->
        <div class="absolute inset-0 bg-black/90 rounded-xl opacity-0 group-hover:opacity-100 transition-opacity flex flex-col items-center justify-center text-[7px] space-y-1">
            <div class="flex justify-between w-full px-2"><span>TEMP:</span><span class="temp-val text-blue-400">--C</span></div>
            <div class="flex justify-between w-full px-2"><span>CURR:</span><span class="curr-val text-yellow-400">--A</span></div>
        </div>
    `;
    grid.appendChild(card);
    antennaCards[i] = {
        element: card,
        dot: card.querySelector('.antenna-dot'),
        label: card.querySelector('.state-label'),
        temp: card.querySelector('.temp-val'),
        curr: card.querySelector('.curr-val')
    };
}

// Websocket logic
let ws;

function connect() {
    ws = new WebSocket('ws://localhost:9001');

    ws.onopen = () => {
        connStatus.innerHTML = `
            <span class="w-2 h-2 rounded-full bg-green-500"></span>
            <span class="font-semibold uppercase text-xs text-green-500">CONNECTED</span>
        `;
        connStatus.className = "flex items-center justify-end gap-2 text-green-500";
    };

    ws.onmessage = (event) => {
        const data = JSON.parse(event.data);
        updateUI(data);
    };

    ws.onclose = () => {
        connStatus.innerHTML = `
            <span class="w-2 h-2 rounded-full bg-red-500 animate-ping"></span>
            <span class="font-semibold uppercase text-xs text-red-500">DISCONNECTED</span>
        `;
        connStatus.className = "flex items-center justify-end gap-2 text-red-500";
        // Reconnect after 3 seconds
        setTimeout(connect, 3000);
    };
}

function updateUI(data) {
    let totalError = 0;
    let faultCount = 0;
    let totalFlux = 0;
    let onlineCount = 0;

    data.forEach(ant => {
        const card = antennaCards[ant.id];
        if (!card) return;

        onlineCount++;
        totalError += ant.error;
        totalFlux += ant.amp;
        if (ant.state === 3) faultCount++;

        // Update dot color
        card.dot.className = `antenna-dot w-3 h-3 rounded-full my-1 transition-colors duration-500 ${ant.state === 0 ? 'bg-gray-500' : ant.state === 1 ? 'bg-yellow-400' : ant.state === 2 ? 'bg-green-400' : 'bg-red-500 shadow-[0_0_10px_rgba(239,68,68,0.5)]'}`;

        const stateNames = ["IDLE", "SLEW", "TRACK", "FAULT"];
        card.label.innerText = stateNames[ant.state];
        card.label.className = `text-[9px] font-mono state-label uppercase ${STATE_COLORS[ant.state]}`;

        // Update Sensor values on Hover
        card.temp.innerText = `${ant.temp.toFixed(1)}C`;
        card.curr.innerText = `${ant.current.toFixed(1)}A`;
    });

    // Update global stats
    stats.active.innerText = `${onlineCount - faultCount} / 50`;
    stats.error.innerText = `${(totalError / (data.length || 1)).toFixed(4)}°`;
    stats.flux.innerText = `${totalFlux.toFixed(2)} Jy`;
    lastUpdateText.innerText = `LAST TELEMETRY: ${new Date().toLocaleTimeString()}`;
    updateTuningChart(data);
}

// Control Input Listeners
const btnDispatch = document.getElementById('btn-dispatch');
const btnReset = document.getElementById('btn-reset');
const inputAz = document.getElementById('cmd-az');
const inputEl = document.getElementById('cmd-el');

btnDispatch.addEventListener('click', () => {
    const az = parseFloat(inputAz.value);
    const el = parseFloat(inputEl.value);
    if (isNaN(az) || isNaN(el)) {
        alert("Please enter valid Azimuth and Elevation coordinates.");
        return;
    }
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({
            action: 'move',
            azimuth: az,
            elevation: el
        }));
        console.log(`Dispatched move command: Az=${az}, El=${el}`);
    } else {
        alert("Not connected to telemetry server.");
    }
});

btnReset.addEventListener('click', () => {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({
            action: 'reset'
        }));
        console.log("Dispatched emergency reset command.");
    } else {
        alert("Not connected to telemetry server.");
    }
});

// Populate Antenna Select list
const pidAntIdSelect = document.getElementById('pid-ant-id');
for (let i = 1; i <= 50; i++) {
    const opt = document.createElement('option');
    opt.value = i;
    opt.innerText = `ANT-${i.toString().padStart(2, '0')}`;
    pidAntIdSelect.appendChild(opt);
}

// Preset Handler
const pidPreset = document.getElementById('pid-preset');
const pidKpInput = document.getElementById('pid-kp');
const pidKiInput = document.getElementById('pid-ki');
const pidKdInput = document.getElementById('pid-kd');

const presets = {
    default: { kp: 1.2, ki: 0.05, kd: 0.3 },
    aggressive: { kp: 2.0, ki: 0.1, kd: 0.5 },
    damped: { kp: 0.8, ki: 0.02, kd: 0.2 },
    sluggish: { kp: 0.5, ki: 0.01, kd: 0.1 }
};

pidPreset.addEventListener('change', () => {
    const p = presets[pidPreset.value];
    if (p) {
        pidKpInput.value = p.kp;
        pidKiInput.value = p.ki;
        pidKdInput.value = p.kd;
    }
});

// Tune dispatch
const btnTunePid = document.getElementById('btn-tune-pid');
btnTunePid.addEventListener('click', () => {
    const antId = parseInt(pidAntIdSelect.value);
    const kp = parseFloat(pidKpInput.value);
    const ki = parseFloat(pidKiInput.value);
    const kd = parseFloat(pidKdInput.value);
    
    if (isNaN(kp) || isNaN(ki) || isNaN(kd)) {
        alert("Please enter valid PID parameters.");
        return;
    }
    
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({
            action: 'tune_pid',
            antennaId: antId,
            kp: kp,
            ki: ki,
            kd: kd
        }));
        console.log(`Dispatched PID tuning: ANT-${antId} Kp=${kp} Ki=${ki} Kd=${kd}`);
    } else {
        alert("Not connected to server.");
    }
});

// Step Test (Autotune button)
const btnAutotune = document.getElementById('btn-autotune');
btnAutotune.addEventListener('click', () => {
    const antId = parseInt(pidAntIdSelect.value);
    let currentAz = 0.0;
    let currentEl = 0.0;
    if (lastTelemetryData && lastTelemetryData.length > 0) {
        const selAnt = lastTelemetryData.find(a => a.id === (antId === 0 ? 1 : antId));
        if (selAnt) {
            currentAz = selAnt.az;
            currentEl = selAnt.el;
        }
    }
    const targetAz = currentAz + 5.0;
    const targetEl = currentEl + 5.0;
    
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({
            action: 'move',
            azimuth: targetAz,
            elevation: targetEl
        }));
        console.log(`Step Test initiated: Az=${targetAz}, El=${targetEl}`);
        errorHistory = [];
    } else {
        alert("Not connected to server.");
    }
});

// Live Chart Logic
const canvas = document.getElementById('response-chart');
const ctx = canvas.getContext('2d');
let errorHistory = [];
let lastTelemetryData = null;

function resizeCanvas() {
    canvas.width = canvas.clientWidth;
    canvas.height = canvas.clientHeight;
}
window.addEventListener('resize', resizeCanvas);
setTimeout(resizeCanvas, 100);

function updateTuningChart(data) {
    lastTelemetryData = data;
    const antId = parseInt(pidAntIdSelect.value);
    const selAnt = data.find(a => a.id === (antId === 0 ? 1 : antId));
    if (!selAnt) return;
    
    errorHistory.push(selAnt.error);
    if (errorHistory.length > 100) {
        errorHistory.shift();
    }
    
    drawChart();
}

function drawChart() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.05)';
    ctx.lineWidth = 1;
    for (let i = 1; i < 4; i++) {
        const y = (canvas.height / 4) * i;
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(canvas.width, y);
        ctx.stroke();
    }
    
    if (errorHistory.length < 2) return;
    
    let maxErr = Math.max(...errorHistory);
    if (maxErr < 1.0) maxErr = 1.0;
    
    ctx.beginPath();
    ctx.strokeStyle = '#a78bfa'; // Purple-400
    ctx.lineWidth = 2;
    
    const dx = canvas.width / (errorHistory.length - 1);
    for (let i = 0; i < errorHistory.length; i++) {
        const x = i * dx;
        const y = canvas.height - (errorHistory[i] / maxErr) * (canvas.height - 10) - 5;
        if (i === 0) {
            ctx.moveTo(x, y);
        } else {
            ctx.lineTo(x, y);
        }
    }
    ctx.stroke();
    
    ctx.fillStyle = 'rgba(255, 255, 255, 0.6)';
    ctx.font = '8px monospace';
    ctx.fillText(`Max Error: ${maxErr.toFixed(2)}°`, 5, 12);
    ctx.fillText(`Current: ${errorHistory[errorHistory.length - 1].toFixed(3)}°`, 5, 22);
}

connect();
