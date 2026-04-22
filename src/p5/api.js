/*
  2026-04-22: DMI humidity example
  - Robust fetch with fallback to local response.json
  - Uses URLSearchParams like Rejseplanen example
  - Normalizes GeoJSON structure
  - Draws humidity + station info
*/

let humidityData = undefined;  // Global variabel til API-data
let errorMessage = '';         // Bruges til at vise fejl i draw()

function setup() {
  createCanvas(560, 420);
  textFont('monospace', 12);

  // Start API-kaldet når sketch starter
  loadHumidity();
}

async function loadHumidity() {
  // Basis-URL til DMI's observations-API
  const baseUrl = 'https://opendataapi.dmi.dk/v2/metObs/collections/observation/items';

  // URLSearchParams → samme stil som i dit Rejseplanen-eksempel
  const params = new URLSearchParams({
    //parameterId: 'humidity',  // Vi vil kun have luftfugtighed
    limit: '1',               // Kun den nyeste observation
    // stationId: '06126'     // (valgfrit) filtrér på specifik station
    
    parameterId: 'wind_speed'
  });
  

  // Kombinér base + querystring
  const url = `${baseUrl}?${params}`;

  try {
    // Forsøg at hente live-data fra DMI
    const response = await fetch(url, {
      method: 'GET',
      headers: {
        Accept: 'application/json'  // Vi vil have JSON tilbage
      }
    });

    // Hvis API'et svarer med fejlstatus (fx 500 eller 404)
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    // Parse JSON-data
    humidityData = await response.json();
    console.log('Live DMI data:', humidityData);

  } catch (e) {
    // Hvis live-kaldet fejler → brug lokal fallback
    console.warn('Live call failed, falling back to local response.json:', e);

    try {
      // Forsøg at hente lokal fil (til debugging)
      const res2 = await fetch('response.json');
      if (!res2.ok) throw new Error(`Local HTTP ${res2.status}`);

      humidityData = await res2.json();
      console.log('Local data:', humidityData);

    } catch (e2) {
      // Hvis både live og lokal fejler → giv op
      console.error('Failed to load local response.json', e2);

      errorMessage = 'Failed to load data (API + local). Open devtools for details.';
      humidityData = null;
    }
  }
}

function draw() {
  background(245);
  fill(20);

  // Stadig i gang med at hente data
  if (humidityData === undefined) {
    text('Loading…', 10, 20);
    return;
  }

  // Fejl i både live og lokal data
  if (humidityData === null) {
    text('Error loading data', 10, 20);
    if (errorMessage) text(errorMessage, 10, 40);
    return;
  }

  // GeoJSON → features-array
  const features = humidityData?.features;
  if (!features || features.length === 0) {
    text('No humidity observations found.', 10, 20);
    return;
  }

  // Tag første observation (den nyeste)
  const obs = features[0];
  const props = obs.properties || {};  // Metadata (værdi, tid, station)
  const geom  = obs.geometry || {};    // Koordinater

  let y = 20;

  // Vis luftfugtighed
  text(`Wind Speed: ${props.value ?? 'n/a'} m/s`, 10, y); 
  y += 20;

  // Vis observationstid
  text(`Observed: ${props.observed || 'n/a'}`, 10, y); 
  y += 20;

  // Vis stationens ID
  text(`Station ID: ${props.stationId || 'n/a'}`, 10, y); 
  y += 20;

  // Vis koordinater hvis de findes
  if (geom.coordinates) {
    text(`Coords: ${geom.coordinates[1]}, ${geom.coordinates[0]}`, 10, y);
  }
}