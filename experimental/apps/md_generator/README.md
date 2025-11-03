# md_generator

**md_generator** is a C++20 command-line application for generating raw, binary market data messages, adhering to exchange protocol. Essentially, md_generator randomly generates market data messages based on parameters that you specify in a `.json` configuration file. Currently, md_generator provides support for the NSDQ (ITCH) protocol. Support for the NYSE (Pillar) and CME (MDP MBO) are coming soon.

## Features

- Generates binary market data messages for multiple symbols.
- Configurable price and quantity ranges per symbol.
- Supports symbol activity percentages (must sum to 100).
- Ensures all orders are in a terminal state at the end of the file.
- Prints progress and summary statistics.

## Usage

```sh
md_generator <config.json> <output.itch>
```

### Arguments

- `<config.json>`: Path to the input configuration file (must be a valid JSON file).
- `<output.itch>`: Path to the output file where generated messages will be saved.

### Example

```sh
./md_generator sample_config.json output.itch
```

## Provided Files

- **`sample_config.json`**:
  - A sample configuration file is provided to help you get started quickly. It contains example symbols, price ranges, and quantity ranges.
  - You can modify this file to suit your specific requirements.

- **`build.sh`**:
  - A sample build script is provided to simplify the build process. It ensures a clean build and sets up all necessary dependencies.

- **`run.sh`**:
  - A sample run script is provided to demonstrate how to execute the application with the provided `sample_config.json` and generate output.

## Config File Format

The configuration file specifies the parameters for generating messages. Below is an example format:

```json
{
  "num_messages": 10000,
  "symbols": [
    {
      "symbol": "MSFT",
      "percent": 60,
      "price_range": { "min_price": 100.0, "max_price": 200.0, "weight": 1.0 },
      "quantity_range": { "min_quantity": 10, "max_quantity": 100, "weight": 1.0 }
    },
    {
      "symbol": "AAPL",
      "percent": 30,
      "price_range": { "min_price": 150.0, "max_price": 250.0, "weight": 1.0 },
      "quantity_range": { "min_quantity": 5, "max_quantity": 50, "weight": 1.0 }
    },
    {
      "symbol": "GOOG",
      "percent": 10,
      "price_range": { "min_price": 1200.0, "max_price": 1300.0, "weight": 1.0 },
      "quantity_range": { "min_quantity": 1, "max_quantity": 10, "weight": 1.0 }
    }
  ]
}
```

### Parameters

- **`num_messages`**:
  - Total number of messages to generate.
  - Must be a positive integer.

- **`symbols`**:
  - An array of symbol configurations. Each symbol must include:
    - **`symbol`**: The name of the symbol (e.g., "MSFT").
    - **`percent`**: The percentage of total messages allocated to this symbol. All percentages must sum to 100.
    - **`price_range`**: Defines the price range for the symbol:
      - **`min_price`**: Minimum price for the range.
      - **`max_price`**: Maximum price for the range.
      - **`weight`**: Weight for selecting this range (currently unused as only one range is allowed).
    - **`quantity_range`**: Defines the quantity range for the symbol:
      - **`min_quantity`**: Minimum quantity for the range.
      - **`max_quantity`**: Maximum quantity for the range.
      - **`weight`**: Weight for selecting this range (currently unused as only one range is allowed).

### Explanation of Behavior

1. **Message Distribution**:
   - The `percent` field determines the relative number of messages generated for each symbol.
   - For example, if `MSFT` has `percent: 60` and `AAPL` has `percent: 30`, then 60% of the messages will be for `MSFT` and 30% for `AAPL`.

2. **Price and Quantity Generation**:
   - Each symbol has exactly one price range and one quantity range.
   - Prices are randomly generated within the specified `min_price` and `max_price`.
   - Quantities are randomly generated within the specified `min_quantity` and `max_quantity`.

3. **Terminal State**:
   - At the end of the file, all orders are closed out with a "Cancel" message to ensure they are in a terminal state.

### Example Output

The generated messages are saved in the specified output file. Below is an example of the output format:

```
Symbol: MSFT, Type: Buy, Quantity: 50, Price: 120.5
Symbol: AAPL, Type: Sell, Quantity: 25, Price: 200.0
Symbol: GOOG, Type: Buy, Quantity: 5, Price: 1250.0
Symbol: MSFT, Type: Cancel, Quantity: 0, Price: 0.0
Symbol: AAPL, Type: Cancel, Quantity: 0, Price: 0.0
Symbol: GOOG, Type: Cancel, Quantity: 0, Price: 0.0
```

## Build

Use the provided build script for a clean build:

```sh
./build.sh
```

## Run

Use the provided run script to execute the application with the sample configuration:

```sh
./run.sh
```

## Notes

- All config validation is performed before generation; errors are printed with hints.
- All generated orders are closed out (cancelled/traded) at the end of the file.
- Timestamps in generated messages are placeholders; playback timing is handled by downstream apps.

## License

No license. Use at your own risk.
