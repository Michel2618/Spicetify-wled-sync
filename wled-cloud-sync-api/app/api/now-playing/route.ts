import { NextResponse } from 'next/server';
import { getAverageColor } from 'fast-average-color-node';

const TOKEN_ENDPOINT = "https://accounts.spotify.com/api/token";
const PLAYER_ENDPOINT = "https://api.spotify.com/v1/me/player/currently-playing";

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

    // --- NEW SERVER-SAFE COLOR EXTRACTION LOGIC ---
    let dominantColor = [255, 255, 255]; 

    if (albumImageUrl) {
      try {
        const color = await getAverageColor(albumImageUrl);
        // The package returns an array like [R, G, B, Opacity], we just grab the first 3!
        dominantColor = [color.value[0], color.value[1], color.value[2]];
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