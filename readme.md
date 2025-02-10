# Real-Time Market Price Simulator

## Overview
A multi-process application that simulates real-time market price updates for various commodities. The system implements a producer-consumer architecture using shared memory and semaphores for inter-process communication. This project demonstrates advanced concepts in concurrent programming and real-time data visualization.

## System Architecture

### Producer Components
- Multiple producer processes generating price updates for different commodities
- Normal distribution-based price generation for realistic market simulation
- Configurable mean and standard deviation for each commodity
- Customizable update intervals

### Consumer Component
- Real-time price display with dynamic updates
- Color-coded price movements (green for increase, red for decrease)
- Rolling average calculations
- Trend indicators (↑ for upward, ↓ for downward)

### Technical Features
- Shared Memory IPC (Inter-Process Communication)
- Semaphore-based synchronization
- Thread-safe operations
- POSIX compliant implementation
- Circular buffer for price history
- Moving average calculations

## Supported Commodities
1. ALUMINIUM
2. COPPER
3. COTTON
4. CRUDEOIL
5. GOLD
6. LEAD
7. MENTHAOIL
8. NATURALGAS
9. NICKEL
10. SILVER
11. ZINC

## Building the Project

```bash
# Build all components
make all

# Build individual components
make producer
make consumer

# Clean build artifacts
make clean
```

## Usage

### Starting the Consumer
```bash
./consumer <buffer-size>
```
Example:
```bash
./consumer 100
```

### Starting Producers
```bash
./producer <commodity-name> <mean-price> <std-deviation> <update-interval> <buffer-size>
```
Example:
```bash
./producer GOLD 1800 10 1000 100
```

## Technical Requirements
- GCC compiler with C++11 support

## Implementation Details

### Shared Memory Structure
- Price history buffer
- Current price indicators
- Moving averages
- Trend indicators
- Synchronization flags

### Synchronization Mechanisms
- Mutex semaphore for exclusive access
- Empty buffer semaphore
- Full buffer semaphore

### Performance Considerations
- Lock-free operations where possible
- Minimal critical sections
- Efficient memory usage
- Optimized display updates

## Contributing
1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Submit a pull request
