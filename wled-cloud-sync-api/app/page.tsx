export default function Home() {
  const CLIENT_ID = process.env.SPOTIFY_CLIENT_ID;
  const REDIRECT_URI = encodeURIComponent(process.env.SPOTIFY_REDIRECT_URI || '');
  const SCOPES = encodeURIComponent('user-read-currently-playing user-read-playback-state');
  
  // Corrected official Spotify authorization URL
  const LOGIN_URL = `https://accounts.spotify.com/authorize?client_id=${CLIENT_ID}&response_type=code&redirect_uri=${REDIRECT_URI}&scope=${SCOPES}`;

  return (
    <main className="flex min-h-screen flex-col items-center justify-center p-24 bg-zinc-950 text-white">
      <div className="max-w-md text-center space-y-6">
        <h1 className="text-3xl font-bold tracking-tight">WLED Cloud Sync Linker</h1>
        <p className="text-zinc-400 text-sm">
          Click the link below to authorize your backend API to fetch your currently playing track color.
        </p>
        <a
          href={LOGIN_URL}
          className="inline-block bg-green-500 hover:bg-green-600 text-black font-semibold px-6 py-3 rounded-full transition-all"
        >
          Connect Spotify Account
        </a>
      </div>
    </main>
  );
}