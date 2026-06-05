import { NextResponse } from 'next/server';
import { getAverageColor } from 'fast-average-color-node';

const TOKEN_ENDPOINT = "https://accounts.spotify.com/api/token";
const PLAYER_ENDPOINT = "https://api.spotify.com/v1/me/player/currently-playing";

// --- COLOR MATH ENGINE ---
// Converts RGB to Hue, Saturation, Lightness
function rgbToHsl(r: number, g: number, b: number) {
  r /= 255; g /= 255; b /= 255;
  const max = Math.max(r, g, b), min = Math.min(r, g, b);
  let h = 0, s = 0, l = (max + min) / 2;

  if (max !== min) {
    const d = max - min;
    s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
    switch (max) {
      case r: h = (g - b) / d + (g < b ? 6 : 0); break;
      case g: h = (b - r) / d + 2; break;
      case b: h = (r - g) / d + 4; break;
    }
    h /= 6;
  }
  return [h, s, l];
}

// Converts Hue, Saturation, Lightness back to RGB for WLED
function hslToRgb(h: number, s: number, l: number) {
  let r, g, b;
  if (s === 0) {
    r = g = b = l; 
  } else {
    const hue2rgb = (p: number, q: number, t: number) => {
      if (t < 0) t += 1;
      if (t > 1) t -= 1;
      if (t < 1/6) return p + (q - p) * 6 * t;
      if (t < 1/2) return q;
      if (t < 2/3) return p + (q - p) * (2/3 - t) * 6;
      return p;
    };
    const q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    const p = 2 * l - q;
    r = hue2rgb(p, q, h + 1/3);
    g = hue2rgb(p, q, h);
    b = hue2rgb(p, q, h - 1/3);
  }
  return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
}
// -------------------------

async function getAccessToken() {
  const clientId = process.env.SPOTIFY_CLIENT_ID;
  const clientSecret = process.env.SPOTIFY_CLIENT_SECRET;
  const refreshToken = process.env.SPOTIFY_REFRESH_TOKEN;

  const response = await fetch(TOKEN_ENDPOINT, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/x-www-form-urlencoded',
      Authorization: 'Basic ' + Buffer.from(clientId + ':' + clientSecret).toString('base64'),
    },
    body: new URLSearchParams({
      grant_type: 'refresh_token',
      refresh_token: refreshToken || '',
    }),
  });

  const data = await response.json();
  return data.access_token;
}

export async function GET() {
  try {
    const accessToken = await getAccessToken();

    const response = await fetch(PLAYER_ENDPOINT, {
      headers: { Authorization: `Bearer ${accessToken}` },
    });

    if (response.status === 204 || response.status > 400) {
      return NextResponse.json({ isPlaying: false, message: "No track currently playing." });
    }

    const song = await response.json();
    const isPlaying = song.is_playing;
    const title = song.item?.name;
    const artist = song.item?.artists.map((_artist: any) => _artist.name).join(', ');
    const albumImageUrl = song.item?.album?.images[0]?.url;

    let dominantColor = [255, 255, 255]; 

    if (albumImageUrl) {
      try {
        const rawColor = await getAverageColor(albumImageUrl, {
          algorithm: 'dominant',
          ignoredColor: [0, 0, 0, 255, 80] // Still ignore pure black
        });
        
        // --- VIBRANCY BOOSTER ---
        let [h, s, l] = rgbToHsl(rawColor.value[0], rawColor.value[1], rawColor.value[2]);
        
        // If the color is not completely grey/black/white
        if (s > 0.05) {
          s = Math.max(s, 0.85); // Force saturation to at least 85% for deep colors
          
          // If the color is too pale (like your pink), darken it slightly to extract the pigment
          if (l > 0.6) l = 0.5; 
          // If it's too dark, brighten it up
          if (l < 0.3) l = 0.4;
        }

        // Convert the boosted color back to RGB for the LED strip
        dominantColor = hslToRgb(h, s, l);
        
      } catch (colorError) {
        console.error("Failed to extract color:", colorError);
      }
    }

    return NextResponse.json({
      isPlaying,
      title,
      artist,
      color: dominantColor,
      albumImageUrl,
    });

  } catch (error: any) {
    return NextResponse.json({ error: "Failed to fetch playback state", details: error.message }, { status: 500 });
  }
}