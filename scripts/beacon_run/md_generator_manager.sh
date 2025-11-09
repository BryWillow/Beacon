#!/bin/bash
# Beacon Market Data Generator Config Manager

CONFIG_DIR="${HOME}/.beacon/md_generator_configs"
GENERATOR_BIN="../../src/apps/exchange_market_data_generator/bin/debug/exchange_market_data_generator"

mkdir -p "$CONFIG_DIR"

usage() {
    echo "Usage:"
    echo "  $0 new <name>      # Create a new config interactively"
    echo "  $0 list            # List available configs"
    echo "  $0 use <name>      # Use (copy) config for generator"
    echo "  $0 run <name>      # Run generator with config"
    exit 1
}

case "$1" in
    new)
        [ -z "$2" ] && usage
        CONFIG_PATH="$CONFIG_DIR/$2.json"
        echo "Creating new config: $CONFIG_PATH"
        # You can replace this with a GUI or interactive prompt
        echo '{"symbols":["AAPL","MSFT"],"burst":1000,"playback_speed":1}' > "$CONFIG_PATH"
        echo "Edit $CONFIG_PATH as needed."
        ;;
    list)
        echo "Available configs:"
        ls "$CONFIG_DIR"
        ;;
    use)
        [ -z "$2" ] && usage
        cp "$CONFIG_DIR/$2.json" "../../src/apps/exchange_market_data_generator/config.json"
        echo "Config $2.json copied to generator."
        ;;
    run)
        [ -z "$2" ] && usage
        "$GENERATOR_BIN" --config "$CONFIG_DIR/$2.json"
        ;;
    *)
        usage
        ;;
esac
