# HexGraphMap

An interactive hexagonal graph editor built with Unreal Engine 5.3. Create, edit, and manipulate hexagonal grid-based graphs with real-time vertex placement, line drawing, and graph piece management.

## Features

- **Interactive Hexagonal Grid**: Place and manipulate vertices on a hexagonal coordinate system
- **Line Drawing Mode**: Connect vertices by drawing lines between them
- **Graph Pieces**: Create and manage reusable graph components
- **Real-time Preview**: Visual feedback during editing operations
- **Camera Controls**: Smooth zoom, pan, and rotation for navigation
- **Save/Load System**: Persist graph data between sessions

## Requirements

- **Unreal Engine 5.3** or later
- **Visual Studio 2022** (for C++ development)
- **Windows 10/11** (primary development platform)

## Getting Started

### Building the Project

1. **Clone the repository**:
   ```bash
   git clone https://github.com/yourusername/HexGraphMap.git
   cd HexGraphMap
   ```

2. **Generate project files**:
   - Right-click on `HexGraphMap.uproject` and select "Generate Visual Studio project files"
   - Or use the command line:
     ```bash
     <UE5InstallPath>\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe -projectfiles -project="HexGraphMap.uproject" -game -rocket -progress
     ```

3. **Build the project**:
   - Open `HexGraphMap.sln` in Visual Studio
   - Build in Development Editor configuration
   - Or launch `HexGraphMap.uproject` directly

### Running the Editor

- Open `HexGraphMap.uproject` in Unreal Engine Editor
- Load the `DevMap` level located in `Content/Maps/`
- Press Play to enter the graph editing mode

## Controls

### Camera Navigation
- **Mouse Wheel**: Zoom in/out
- **WASD**: Pan camera
- **R**: Rotate view

### Graph Editing
- **Left Click**: Select/place vertices
- **Right Click**: Context actions
- **Line Draw Mode**: Hold to draw connections between vertices
- **Delete**: Remove selected vertices
- **Fill**: Create connected vertex groups

## Project Structure

```
HexGraphMap/
├── Content/
│   ├── Blueprints/          # Blueprint assets for UI and game logic
│   ├── Maps/                # Level files
│   ├── Inputs/              # Enhanced Input mappings
│   └── Meshes/              # 3D models for vertices
├── Source/
│   └── HexGraphMap/         # C++ source code
│       ├── HexGraph.*       # Main graph controller
│       ├── Vertex.*         # Base vertex class
│       ├── GraphVertex.*    # Concrete vertex implementation
│       └── AdjacencyMap.*   # Hexagonal adjacency management
└── Config/                  # Project configuration files
```

## Architecture

The project implements a three-tier vertex system:

- **AVertex** (Abstract): Base vertex functionality with positioning and type management
- **AGraphVertex**: Concrete vertex implementations for actual graph nodes  
- **APlaceHolderVertex**: Temporary/preview vertices for editing operations

The hexagonal coordinate system uses row/column integers internally, with string-based coordinates for serialization and lookup.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes following Unreal Engine C++ coding standards
4. Test your changes in the editor
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Development Notes

- Uses Unreal Engine's Enhanced Input system for all input handling
- Implements custom adjacency matrix for hexagonal topology
- Supports temporary vertex placement with commit/rollback functionality
- Camera system uses spring arm component for smooth navigation