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
}

connect();
