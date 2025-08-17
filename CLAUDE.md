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

## Documentation and Knowledge Management

### Obsidian Vault Integration

**All Claude-generated documentation, plans, and analysis should be created in the Obsidian vault located at:**
`HexGraphGame/Claude-Generated/`

### Vault Structure
```
HexGraphGame/Claude-Generated/
├── Plans/           # Implementation plans, roadmaps, task lists
├── Analysis/        # Code analysis, architecture reviews
├── Architecture/    # Architectural decision records, design docs
└── Documentation/   # API docs, guides, tutorials
```

### Documentation Standards for Claude Code

When creating documentation in the Obsidian vault:

1. **Use Obsidian Linking**: Always link related concepts using `[[Concept Name]]` syntax
2. **Tag Content**: Use tags like `#architecture`, `#refactoring`, `#analysis`, `#claude-generated`
3. **Cross-Reference**: Link to related files, classes, and concepts throughout documents
4. **Metadata Headers**: Include project info, creation date, status, and document type
5. **Structured Formatting**: Use consistent heading hierarchy and formatting

### Key Concepts to Link
- `[[HexGraphMap]]` - Main project
- `[[HexGraph]]` - Core controller class
- `[[Vertex]]` - Vertex system and hierarchy
- `[[Adjacency Map]]` - Adjacency management system
- `[[Enhanced Input]]` - Input system
- `[[Camera Controls]]` - Camera management
- `[[Coordinate System]]` - Hexagonal grid coordinates
- `[[Line Drawing]]` - Drawing and preview systems
- `[[Memory Management]]` - Object lifecycle and GC
- `[[Performance]]` - Performance considerations
- `[[Architecture]]` - System architecture concepts

### Document Templates

**Analysis Documents:**
```markdown
# Title
> **Project:** [[HexGraphMap]]
> **Type:** #analysis #architecture
> **Status:** #active
> **Created:** YYYY-MM-DD
> **Analyzed By:** Claude Code

[Content with extensive linking to related concepts]

**Tags:** #relevant #tags #claude-generated
```

**Implementation Plans:**
```markdown
# Title
> **Project:** [[HexGraphMap]]
> **Type:** #implementation-plan
> **Status:** #active
> **Total Tasks:** N
> **Dependencies:** [[Related Documents]]

[Detailed task breakdown with links to architectural concepts]

**Tags:** #implementation #planning #claude-generated
```

### Best Practices

1. **Always check if an Obsidian vault exists** before creating documentation
2. **Use the vault structure** to organize different types of documents
3. **Link extensively** to create a knowledge graph of project concepts
4. **Update existing documents** when new information becomes available
5. **Reference the vault location** in code comments when relevant

### Integration with Development

- Reference Obsidian documentation in code comments when appropriate
- Link to specific analysis or architectural documents in pull requests
- Use the vault as the primary location for all Claude-generated project knowledge
- Keep documentation synchronized with code changes