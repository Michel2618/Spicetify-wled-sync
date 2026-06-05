// NAME: WLED Sync
// AUTHOR: Michel Ruwishka
// DESCRIPTION: Syncs WLED strips with Spotify album art colors using local HTTP requests.

(function WLED() {
    console.log("[WLED] Event Listener Script Loaded.");

    // ------------------------------------------
    // CONFIGURATION
    // ------------------------------------------
    // CHANGE THIS: The IP address of your WLED controller 
    
    const WLED_IP = "YOUR_WLED_IP_HERE"; // e.g., "192.168.1.15"
    const BRIGHTNESS = 255;
    // ------------------------------------------

    let lastSentColor = "";

    async function sendToWLED(r, g, b) {
        // Optimization: Don't send the exact same color twice to save Wi-Fi bandwidth
        let colorCode = `${r},${g},${b}`;
        if (colorCode === lastSentColor) return;
        lastSentColor = colorCode;

        try {
            await fetch(`http://${WLED_IP}/json/state`, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({
                    "on": true,
                    "bri": BRIGHTNESS,
                    "transition": 10, // Smooth transition (1 second)
                    "seg": [{ "col": [[r, g, b]] }]
                })
            });
            console.log(`[WLED] Sent Color: ${r}, ${g}, ${b}`);
        } catch (e) {
            console.error("[WLED] Connection Error. Check WLED IP.");
        }
    }

    function getVibrantColor(img) {
        let canvas = document.createElement("canvas");
        let ctx = canvas.getContext("2d");
        canvas.width = 50;
        canvas.height = 50;
        ctx.drawImage(img, 0, 0, 50, 50);

        let data = ctx.getImageData(0, 0, 50, 50).data;
        let r = 0, g = 0, b = 0, count = 0;

        for (let i = 0; i < data.length; i += 4) {
            r += data[i];
            g += data[i+1];
            b += data[i+2];
            count++;
        }

        // Calculate Average
        r = Math.floor(r / count);
        g = Math.floor(g / count);
        b = Math.floor(b / count);

        // Color Correction: Fix washed-out (white) colors
        let min = Math.min(r, g, b);
        let max = Math.max(r, g, b);
        if (max - min < 200) {
            r -= min; g -= min; b -= min;
            let scale = 255 / (Math.max(r, g, b) || 1);
            r = Math.floor(r * scale);
            g = Math.floor(g * scale);
            b = Math.floor(b * scale);
        }
        return {r, g, b};
    }

    function processNowPlaying() {
        // Target the stable "Now Playing" widget in the sidebar
        let img = document.querySelector(".main-nowPlayingWidget-coverArt .main-image-image");
        if (!img) return;

        let rawImg = new Image();
        rawImg.crossOrigin = "Anonymous";
        rawImg.src = img.src;

        rawImg.onload = function() {
            let c = getVibrantColor(rawImg);
            // Saturation Filter: Ignore gray/white images
            let sat = Math.max(c.r, c.g, c.b) - Math.min(c.r, c.g, c.b);
            if (sat > 20) {
                sendToWLED(c.r, c.g, c.b);
            }
        };
    }

    // Trigger: Listen for the internal Spicetify 'songchange' event
    function onSongChange() {
        // Wait 1 second for the new cover art to render in the DOM
        setTimeout(processNowPlaying, 1000);
        setTimeout(processNowPlaying, 2000); // Double check for slow connections
    }

    if (Spicetify.Player) {
        Spicetify.Player.addEventListener("songchange", onSongChange);
        processNowPlaying();
    } else {
        setTimeout(() => {
             if (Spicetify.Player) Spicetify.Player.addEventListener("songchange", onSongChange);
        }, 3000);
    }
})();