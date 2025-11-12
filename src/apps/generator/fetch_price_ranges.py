#!/usr/bin/env python3
"""
Fetch previous day's high/low prices for symbols to generate realistic market data.
Falls back to sensible defaults if API fetch fails.
"""

import json
import sys
from pathlib import Path
from typing import Dict, Any
from urllib.request import urlopen
from urllib.error import URLError
import urllib.parse

# Realistic default values (based on typical Nov 2024 prices)
DEFAULT_RANGES = {
    "AAPL": {
        "symbol": "AAPL",
        "reference_price": 178.50,
        "min_price": 175.00,
        "max_price": 182.00,
        "note": "Default range (±2% typical intraday)"
    },
    "MSFT": {
        "symbol": "MSFT",
        "reference_price": 380.00,
        "min_price": 375.00,
        "max_price": 385.00,
        "note": "Default range (±1.5% typical intraday)"
    },
    "TSLA": {
        "symbol": "TSLA",
        "reference_price": 242.00,
        "min_price": 235.00,
        "max_price": 250.00,
        "note": "Default range (±3% typical intraday, more volatile)"
    },
    "GOOGL": {
        "symbol": "GOOGL",
        "reference_price": 140.00,
        "min_price": 138.00,
        "max_price": 142.00,
        "note": "Default range (±1.5% typical intraday)"
    },
    "AMZN": {
        "symbol": "AMZN",
        "reference_price": 178.00,
        "min_price": 175.00,
        "max_price": 181.00,
        "note": "Default range (±2% typical intraday)"
    }
}

def fetch_yahoo_finance_data(symbol: str) -> Dict[str, Any]:
    """
    Fetch previous day's OHLC data from Yahoo Finance (no API key needed).
    Uses urllib (built-in) instead of requests for no dependencies.
    
    Returns dict with: previousClose, low, high, open
    Raises exception if fetch fails.
    """
    import time
    from urllib.request import Request
    
    base_url = "https://query1.finance.yahoo.com/v8/finance/chart/"
    params = urllib.parse.urlencode({
        "interval": "1d",
        "range": "5d"  # Get last 5 days to ensure we have data
    })
    url = f"{base_url}{symbol}?{params}"
    
    # Add headers to look like a real browser (helps avoid rate limiting)
    headers = {
        'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36',
        'Accept': 'application/json'
    }
    
    try:
        # Add small delay between requests to avoid rate limiting
        time.sleep(0.5)
        
        req = Request(url, headers=headers)
        with urlopen(req, timeout=10) as response:
            data = json.loads(response.read().decode('utf-8'))
        
        # Extract quote data
        result = data['chart']['result'][0]
        meta = result['meta']
        
        # Get the most recent complete trading day
        quotes = result['indicators']['quote'][0]
        
        # Yahoo returns arrays, take the last complete day (not today)
        # If today is trading day, last index might be incomplete, use [-2]
        idx = -2 if len(quotes['close']) > 1 else -1
        
        return {
            "previousClose": meta.get('previousClose', meta.get('regularMarketPrice')),
            "low": quotes['low'][idx],
            "high": quotes['high'][idx],
            "open": quotes['open'][idx],
            "volume": quotes['volume'][idx]
        }
    except (URLError, KeyError, IndexError, json.JSONDecodeError) as e:
        raise Exception(f"Failed to fetch {symbol}: {e}")


def generate_price_range(symbol: str) -> Dict[str, Any]:
    """
    Generate price range for a symbol.
    Try to fetch from Yahoo Finance, fall back to defaults if it fails.
    """
    try:
        print(f"  Fetching {symbol}...", end=" ", flush=True)
        data = fetch_yahoo_finance_data(symbol)
        
        # Use previous day's actual high/low
        min_price = data['low']
        max_price = data['high']
        reference_price = data['previousClose']
        
        print(f"✓ ${reference_price:.2f} (range: ${min_price:.2f} - ${max_price:.2f})")
        
        return {
            "symbol": symbol,
            "reference_price": reference_price,
            "min_price": min_price,
            "max_price": max_price,
            "previous_close": data['previousClose'],
            "previous_open": data['open'],
            "previous_volume": data['volume'],
            "source": "Yahoo Finance (previous day)"
        }
        
    except Exception as e:
        print(f"✗ Failed, using defaults")
        print(f"    Error: {e}")
        
        # Fall back to default values
        if symbol in DEFAULT_RANGES:
            return DEFAULT_RANGES[symbol]
        else:
            # Generic fallback for unknown symbols
            return {
                "symbol": symbol,
                "reference_price": 100.00,
                "min_price": 95.00,
                "max_price": 105.00,
                "note": "Default range (symbol not in fallback list)"
            }


def update_config_file(output_data: Dict[str, Any], config_path: Path):
    """
    Update the market data generator config file with new price ranges.
    Preserves other settings, only updates symbol price ranges.
    """
    try:
        # Read existing config if it exists
        if config_path.exists():
            with open(config_path, 'r') as f:
                config = json.load(f)
        else:
            # Create minimal config structure
            config = {
                "output_file": "output.itch",
                "message_count": 10000,
                "symbols": []
            }
        
        # Update symbols with new price ranges
        symbol_map = {s['symbol']: s for s in output_data['symbols']}
        
        # Update existing symbols or add new ones
        existing_symbols = {s['symbol'] for s in config.get('symbols', [])}
        
        for symbol_data in output_data['symbols']:
            symbol = symbol_data['symbol']
            if symbol in existing_symbols:
                # Update existing symbol
                for s in config['symbols']:
                    if s['symbol'] == symbol:
                        s['min_price'] = symbol_data['min_price']
                        s['max_price'] = symbol_data['max_price']
                        s['reference_price'] = symbol_data['reference_price']
                        break
            else:
                # Add new symbol with defaults
                config['symbols'].append({
                    "symbol": symbol,
                    "min_price": symbol_data['min_price'],
                    "max_price": symbol_data['max_price'],
                    "reference_price": symbol_data['reference_price'],
                    "tick_frequency": 1.0,
                    "volatility": 0.02
                })
        
        # Write updated config
        with open(config_path, 'w') as f:
            json.dump(config, f, indent=2)
        
        print(f"\n✓ Updated config file: {config_path}")
        return True
        
    except Exception as e:
        print(f"\n✗ Failed to update config file: {e}")
        return False


def main():
    print("═" * 70)
    print("  Fetching Previous Day Price Ranges")
    print("═" * 70)
    print()
    
    # Default symbols to fetch
    symbols = ["AAPL", "MSFT", "TSLA", "GOOGL", "AMZN"]
    
    # Allow custom symbols from command line
    if len(sys.argv) > 1:
        symbols = sys.argv[1].split(',')
        print(f"Using custom symbols: {', '.join(symbols)}")
        print()
    
    # Fetch data for each symbol
    output_data = {
        "symbols": [],
        "fetch_timestamp": None,
        "source": "Yahoo Finance with fallback to defaults"
    }
    
    for symbol in symbols:
        range_data = generate_price_range(symbol)
        output_data['symbols'].append(range_data)
    
    print()
    print("─" * 70)
    print("  Summary")
    print("─" * 70)
    
    # Count successful vs fallback
    successful = sum(1 for s in output_data['symbols'] if 'source' in s and 'Yahoo' in s['source'])
    fallback = len(output_data['symbols']) - successful
    
    print(f"  Successfully fetched: {successful}/{len(symbols)}")
    if fallback > 0:
        print(f"  Using fallback:       {fallback}/{len(symbols)}")
    print()
    
    # Save to output file
    script_dir = Path(__file__).parent
    output_file = script_dir / "price_ranges.json"
    
    try:
        with open(output_file, 'w') as f:
            json.dump(output_data, f, indent=2)
        print(f"✓ Saved to: {output_file}")
    except Exception as e:
        print(f"✗ Failed to save output file: {e}")
        return 1
    
    # Also update sample_config.json if it exists
    config_file = script_dir / "sample_config.json"
    if config_file.exists():
        print()
        print(f"Updating {config_file.name}...")
        update_config_file(output_data, config_file)
    
    print()
    print("═" * 70)
    print("  Complete!")
    print("═" * 70)
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
