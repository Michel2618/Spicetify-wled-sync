# WLED Cloud Sync API

This directory contains the headless, serverless backend that bridges Spotify's music data with your WLED ambient light system. Built with **Next.js**, it handles OAuth 2.0 authentication and real-time color extraction from album artwork.

## Project Architecture
1. **OAuth Flow (`/api/callback`):** Handles the initial Spotify authorization and generates a long-lived Refresh Token.
2. **Data Fetching (`/api/now-playing`):** Polls the Spotify API, extracts the current song metadata, and uses `fast-average-color-node` to calculate vibrant RGB values.
3. **Color Engine:** Includes a custom HSL-based vibrancy booster that ignores dark backgrounds and saturates pale colors to ensure your LEDs look punchy and vibrant.

## Deployment Requirements
This API is designed to run on **Vercel**. To deploy successfully, you must configure the following Environment Variables in your Vercel Project Settings:

| Key | Description |
| :--- | :--- |
| `SPOTIFY_CLIENT_ID` | Your Spotify Developer Dashboard Client ID |
| `SPOTIFY_CLIENT_SECRET` | Your Spotify Developer Dashboard Client Secret |
| `SPOTIFY_REDIRECT_URI` | `https://your-project.vercel.app/api/callback` |
| `SPOTIFY_REFRESH_TOKEN` | The persistent token generated during local setup |

## Local Development
1. Clone the repository and navigate to this folder.
2. Create a `.env.local` file with the keys listed above.
3. Run the development server:
   ```bash
   npm install
   npm run dev 
   ```
4. Access the API locally at http://127.0.0.1:3000/api/now-playing.

## Security Note
This project utilizes .gitignore to prevent your sensitive Spotify credentials from being committed to GitHub. Always ensure your environment variables are managed securely within Vercel's protected settings.