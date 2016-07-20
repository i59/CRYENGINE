# Template Description
This directory containg the source code for the Top Down Shooter template originally based on GameZero.

The sample implements a top down view and movement using the WASD keys, along with basic shooting functionality.

## Future Extractions
The sample aims to extract additional logic into the engine itself, in order to minimize the amount of code required to get started.

Logic intended to be moved out are as follows:
* CEditorGame
* CNativeEntityBase
* CNativeEntityPropertyHandler
* CEnvironmentProbeEntity
* CLightEntity
* CFlowGameEntityNode
* CGameFactory helpers
* StringConversions.h