import { NextResponse } from 'next/server';

export async function GET(request: Request) {
  const { searchParams } = new URL(request.url);
  const code = searchParams.get('code');
  const error = searchParams.get('error');

  if (error) {
    return NextResponse.json({ error: `Spotify Auth Error: ${error}` }, { status: 400 });
  }

  if (!code) {
    return NextResponse.json({ error: 'No authorization code found.' }, { status: 400 });
  }

  const clientId = process.env.SPOTIFY_CLIENT_ID;
  const clientSecret = process.env.SPOTIFY_CLIENT_SECRET;
  const redirectUri = process.env.SPOTIFY_REDIRECT_URI;

  try {
    const response = await fetch('https://accounts.spotify.com/api/token', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded',
        Authorization: 'Basic ' + Buffer.from(clientId + ':' + clientSecret).toString('base64'),
      },
      body: new URLSearchParams({
        grant_type: 'authorization_code',
        code: code,
        redirect_uri: redirectUri || '',
      }),
    });

    const data = await response.json();

    if (!response.ok) {
      return NextResponse.json({ error: 'Failed to exchange token', details: data }, { status: response.status });
    }

    return new NextResponse(`
      <html>
        <body style="background: #09090b; color: white; font-family: sans-serif; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0; padding: 20px; text-align: center;">
          <div style="background: #18181b; padding: 30px; border-radius: 12px; max-width: 600px; word-break: break-all; border: 1px solid #27272a;">
            <h1 style="color: #22c55e; margin-top: 0;">Success! Connected to Spotify</h1>
            <p style="color: #a1a1aa; font-size: 14px;">Copy the code below. This is your permanent <b>Refresh Token</b>. Save it safely—we will add this to Vercel later.</p>
            <textarea readonly style="width: 100%; height: 80px; background: #09090b; color: #22c55e; border: 1px solid #3f3f46; padding: 10px; border-radius: 6px; font-family: monospace; resize: none; margin-top: 15px;" onclick="this.select()">${data.refresh_token}</textarea>
            <p style="color: #71717a; font-size: 12px; margin-bottom: 0; margin-top: 15px;">Once copied, you can close this tab and stop your local terminal server.</p>
          </div>
        </body>
      </html>
    `, { headers: { 'Content-Type': 'text/html' } });

  } catch (err) {
    return NextResponse.json({ error: 'Internal Server Error', details: err }, { status: 500 });
  }
}