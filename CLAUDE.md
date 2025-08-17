# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

HexGraphMap is an Unreal Engine 5.3 project that implements a hexagonal graph editor. The project allows users to create, edit, and manipulate hexagonal grid-based graphs with interactive vertex placement, line drawing, and graph piece management capabilities.

## Development Commands

### Building the Project
- **Primary Build**: Open `HexGraphMap.sln` in Visual Studio and build the solution
- **Unreal Editor**: Launch `HexGraphMap.uproject` directly in Unreal Engine Editor
- **Command Line Build**: Use UnrealBuildTool (UBT) commands for automated builds:
  ```
  <UE5InstallPath>\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe HexGraphMapEditor Win64 Development
  ```

### Testing
- **In-Editor Testing**: Use Unreal Editor's Play-in-Editor (PIE) functionality
- **Packaged Testing**: Package the project for testing standalone builds

## Code Architecture

### Core Systems

**HexGraph (AHexGraph)** - Main pawn controller managing the hexagonal graph system
- Handles vertex creation, placement, and management
- Manages input mapping and camera controls
- Coordinates line drawing and graph piece selection
- Located at: `Source/HexGraphMap/HexGraph.h/.cpp`

**Vertex Hierarchy** - Three-tier vertex system:
- `AVertex` (Abstract base) - Core vertex functionality with positioning and type management
- `AGraphVertex` - Concrete vertex implementations for actual graph nodes
- `APlaceHolderVertex` - Temporary/preview vertices for editing operations

**Adjacency Management** - Hexagonal neighbor tracking system:
- `UAdjacencyMap` - Stores and manages vertex connections
- `EHexagonDirection` - Enum defining 6 hexagonal directions (North, Northeast, Southeast, South, Southwest, Northwest)
- Coordinate system uses row/col integers converted to string format

**Input System** - Enhanced Input system integration:
- Camera controls (zoom, pan, rotate)
- Vertex selection and manipulation
- Line drawing mode for connecting vertices
- Fill operations for batch vertex creation

### Key Components

**HexGraphEditorPlayerController** - Specialized player controller for graph editing
**GraphEditorHUD** - UI system for graph editing interface
**Mouse Follower System** - Debug visualization for cursor tracking
**Save/Load System** - Graph persistence through Unreal's save game system

### Coordinate System

The project uses a dual coordinate system:
- **Row/Col integers** - Internal mathematical representation
- **String coordinates** - Serializable format for storage and lookup
- Conversion handled by `intsToCoordString()` and `coordStringToInts()`

### Vertex Types

Defined in `EVertexType` enum:
- `Vertex` - Standard vertex
- `Graph` - Graph node vertex  
- `PlaceHolder` - Temporary preview vertex
- `Preview` - Real-time preview during operations

## Module Structure

**Primary Module**: `HexGraphMap`
- **Type**: Runtime module
- **Dependencies**: Core, CoreUObject, Engine, InputCore, EnhancedInput
- **Target Platform**: Windows 64-bit (extensible to other platforms)

## Content Organization

- **Blueprints/**: Blueprint assets for graph components and UI
- **Maps/**: Development and testing levels
- **Inputs/**: Enhanced Input mapping contexts and actions
- **Meshes/**: 3D models for vertex visualization

## Development Notes

- Uses Unreal Engine's Enhanced Input system for all input handling
- Implements custom adjacency matrix for hexagonal topology
- Supports temporary vertex placement with commit/rollback functionality
- Camera system uses spring arm component for smooth navigation
- Vertex spacing and mesh dimensions are configurable properties