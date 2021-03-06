// ==============================================================
//	This file is part of Glest (www.glest.org)
//
//	Copyright (C) 2001-2008 Martiño Figueroa
//
//	You can redistribute this code and/or modify it under
//	the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the
//	License, or (at your option) any later version
// ==============================================================

#ifndef _GLEST_GAME_RENDERER_H_
#define _GLEST_GAME_RENDERER_H_

#ifdef WIN32
    #include <winsock2.h>
    #include <winsock.h>
#endif

#include "vec.h"
#include "math_util.h"
#include "model.h"
#include "particle.h"
#include "pixmap.h"
#include "font.h"
#include "matrix.h"
#include "selection.h"
#include "components.h"
#include "texture.h"
#include "model_manager.h"
#include "graphics_factory_gl.h"
#include "font_manager.h"
#include "camera.h"
#include <vector>
#include "model_renderer.h"
#include "model.h"
#include "graphics_interface.h"
#include "base_renderer.h"
#include "simple_threads.h"
#include "video_player.h"

#ifdef DEBUG_RENDERING_ENABLED
#	define IF_DEBUG_EDITION(x) x
#	include "debug_renderer.h"
#else
#	define IF_DEBUG_EDITION(x)
#endif

#include "leak_dumper.h"

enum DebugUILevelType {
	debugui_fps 		= 0x01,
	debugui_unit_titles = 0x02
};

namespace Glest{ namespace Game{

using namespace ::Shared::Graphics;
using namespace ::Shared::PlatformCommon;

//non shared classes
class Config;
class Game;
class GameCamera;
class MainMenu;
class Console;
class MenuBackground;
class ChatManager;
class Object;
class ConsoleLineInfo;
class SurfaceCell;
class Program;
// =====================================================
// 	class MeshCallbackTeamColor
// =====================================================

class MeshCallbackTeamColor: public MeshCallback {
private:
	const Texture *teamTexture;

public:
	MeshCallbackTeamColor() : MeshCallback() {
		teamTexture = NULL;
	}
	void setTeamTexture(const Texture *teamTexture)	{this->teamTexture= teamTexture;}
	virtual void execute(const Mesh *mesh);

	static bool noTeamColors;
};

// ===========================================================
// 	class Renderer
//
///	OpenGL renderer, uses the shared library
// ===========================================================

class VisibleQuadContainerCache {
protected:

	inline void CopyAll(const VisibleQuadContainerCache &obj) {
		cacheFrame 			= obj.cacheFrame;
		visibleObjectList	= obj.visibleObjectList;
		visibleUnitList		= obj.visibleUnitList;
		visibleQuadUnitList = obj.visibleQuadUnitList;
		visibleQuadUnitBuildList = obj.visibleQuadUnitBuildList;
		visibleScaledCellList = obj.visibleScaledCellList;
		visibleScaledCellToScreenPosList = obj.visibleScaledCellToScreenPosList;
		lastVisibleQuad		= obj.lastVisibleQuad;
		frustumData			= obj.frustumData;
		proj				= obj.proj;
		modl				= obj.modl;
		frustumDataCache	= obj.frustumDataCache;
	}

public:

	inline VisibleQuadContainerCache() {
		cacheFrame = 0;
		clearFrustumData();
		clearCacheData();
	}
	inline VisibleQuadContainerCache(const VisibleQuadContainerCache &obj) {
		CopyAll(obj);
	}
	inline VisibleQuadContainerCache & operator=(const VisibleQuadContainerCache &obj) {
		CopyAll(obj);
		return *this;
	}

	inline void clearCacheData() {
		clearVolatileCacheData();
		clearNonVolatileCacheData();
	}
	inline void clearVolatileCacheData() {
		visibleUnitList.clear();
		visibleQuadUnitList.clear();
		visibleQuadUnitBuildList.clear();
		//inVisibleUnitList.clear();

		visibleUnitList.reserve(500);
		visibleQuadUnitList.reserve(500);
		visibleQuadUnitBuildList.reserve(100);
	}
	inline void clearNonVolatileCacheData() {
		visibleObjectList.clear();
		visibleScaledCellList.clear();
		visibleScaledCellToScreenPosList.clear();

		visibleObjectList.reserve(500);
		visibleScaledCellList.reserve(500);
	}
	inline void clearFrustumData() {
		frustumData = vector<vector<float> >(6,vector<float>(4,0));
		proj = vector<float>(16,0);
		modl = vector<float>(16,0);
		frustumDataCache.clear();
	}
	int cacheFrame;
	Quad2i lastVisibleQuad;
	std::vector<Object *> visibleObjectList;
	std::vector<Unit   *> visibleQuadUnitList;
	std::vector<UnitBuildInfo> visibleQuadUnitBuildList;
	std::vector<Unit   *> visibleUnitList;
	std::vector<Vec2i> visibleScaledCellList;
	std::map<Vec2i,Vec3f> visibleScaledCellToScreenPosList;

	static bool enableFrustumCalcs;
	vector<vector<float> > frustumData;
	vector<float> proj;
	vector<float> modl;
	map<pair<vector<float>,vector<float> >, vector<vector<float> > > frustumDataCache;

};

class VisibleQuadContainerVBOCache {
public:
	// Vertex Buffer Object Names
	bool    hasBuiltVBOs;
	uint32	m_nVBOVertices;					// Vertex VBO Name
	uint32	m_nVBOFowTexCoords;				// Texture Coordinate VBO Name for fog of war texture coords
	uint32	m_nVBOSurfaceTexCoords;			// Texture Coordinate VBO Name for surface texture coords
	uint32	m_nVBONormals;					// Normal VBO Name
	//uint32	m_nVBOIndexes;					// Indexes VBO Name
};


class Renderer : public RendererInterface,
				 public BaseRenderer,
				 // This is for screen saver thread
				 public SimpleTaskCallbackInterface,
				 public VideoLoadingCallbackInterface {
public:
	//progress bar
	static const int maxProgressBar;
	static const Vec4f progressBarBack1;
	static const Vec4f progressBarBack2;
	static const Vec4f progressBarFront1;
	static const Vec4f progressBarFront2;

	//sun and moon
	static const float sunDist;
	static const float moonDist;
	static const float lightAmbFactor;

	//mouse
	static const int maxMouse2dAnim;

	//texture units
	static const GLenum baseTexUnit;
	static const GLenum fowTexUnit;
	static const GLenum shadowTexUnit;

	//selection
	static const float selectionCircleRadius;
	static const float magicCircleRadius;

	//perspective values
	static const float perspFov;
	static const float perspNearPlane;
	static float perspFarPlane;

	//default values
	static const float ambFactor;
	static const Vec4f defSpecularColor;
	static const Vec4f defDiffuseColor;
	static const Vec4f defAmbientColor;
	static const Vec4f defColor;
	static const Vec4f fowColor;

	//light
	static const float maxLightDist;

	static bool renderText3DEnabled;

public:
	enum Shadows {
		sDisabled,
		sProjected,
		sShadowMapping,

		sCount
	};

private:

	//config
	int maxLights;
    bool photoMode;
	int shadowTextureSize;
	int shadowFrameSkip;
	float shadowIntensity;
	bool focusArrows;
	bool textures3D;
	Shadows shadows;
	int maxConsoleLines;

	//game
	const Game *game;
	GameCamera *gameCamera;
	const MainMenu *menu;
	Program *program;

	//misc
	int triangleCount;
	int pointCount;
	Quad2i visibleQuad;
	Quad2i visibleQuadFromCamera;
	Vec4f nearestLightPos;
	VisibleQuadContainerCache quadCache;
	VisibleQuadContainerCache quadCacheSelection;

	//renderers
	ModelRenderer *modelRenderer;
	TextRenderer2D *textRenderer;
	TextRenderer3D *textRenderer3D;
	ParticleRenderer *particleRenderer;

	//texture managers
	ModelManager *modelManager[rsCount];
	TextureManager *textureManager[rsCount];
	FontManager *fontManager[rsCount];
	ParticleManager *particleManager[rsCount];

	//state lists
	//GLuint list3d;
	//bool list3dValid;
	//GLuint list2d;
	//bool list2dValid;
	//GLuint list3dMenu;
	//bool list3dMenuValid;
	//GLuint *customlist3dMenu;
	//const MainMenu *mm3d;
	const MainMenu *custom_mm3d;

	//shadows
	GLuint shadowMapHandle;
	bool shadowMapHandleValid;

	Matrix4f shadowMapMatrix;
	int shadowMapFrame;

	//water
	float waterAnim;

	bool allowRenderUnitTitles;
	//std::vector<std::pair<Unit *,Vec3f> > renderUnitTitleList;
	std::vector<Unit *> visibleFrameUnitList;
	string visibleFrameUnitListCameraKey;

	bool no2DMouseRendering;
	bool showDebugUI;
	int showDebugUILevel;

	int lastRenderFps;
	float smoothedRenderFps;
	bool shadowsOffDueToMinRender;

	std::vector<std::pair<ParticleSystem *, ResourceScope> > deferredParticleSystems;

	SimpleTaskThread *saveScreenShotThread;
	Mutex *saveScreenShotThreadAccessor;
	std::list<std::pair<string,Pixmap2D *> > saveScreenQueue;

	std::map<Vec3f,Vec3f> worldToScreenPosCache;

	//bool masterserverMode;

	std::map<uint32,VisibleQuadContainerVBOCache > mapSurfaceVBOCache;

	class SurfaceData {
	public:
		inline SurfaceData() {
			uniqueId=0;
			bufferCount=0;
			textureHandle=0;
		}
		static uint32 nextUniqueId;
		uint32 uniqueId;
		int bufferCount;
		int textureHandle;
		vector<Vec2f> texCoords;
		vector<Vec2f> texCoordsSurface;
		vector<Vec3f> vertices;
		vector<Vec3f> normals;
	};

	VisibleQuadContainerVBOCache * GetSurfaceVBOs(SurfaceData *cellData);
	void ReleaseSurfaceVBOs();
	std::map<string,std::pair<Chrono, std::vector<SurfaceData> > > mapSurfaceData;
	static bool rendererEnded;
	
	class MapRenderer {
	public:
		inline MapRenderer(): map(NULL) {}
		inline ~MapRenderer() { destroy(); }
		void render(const Map* map,float coordStep,VisibleQuadContainerCache &qCache);
		void renderVisibleLayers(const Map* map,float coordStep,VisibleQuadContainerCache &qCache);
		void destroy();
	private:
		void load(float coordStep);
		void loadVisibleLayers(float coordStep,VisibleQuadContainerCache &qCache);

		const Map* map;
		struct Layer {
			inline Layer(int th):
				vbo_vertices(0), vbo_normals(0), 
				vbo_fowTexCoords(0), vbo_surfTexCoords(0),
				vbo_indices(0), indexCount(0),
				textureHandle(th),textureCRC(0) {}

			inline Layer & operator=(Layer &obj) {
				this->vertices = obj.vertices;
				this->normals = obj.normals;
				this->fowTexCoords = obj.fowTexCoords; 
				this->surfTexCoords = obj.surfTexCoords;
				this->indices = obj.indices;
				this->cellToIndicesMap = obj.cellToIndicesMap;
				this->rowsToRenderCache = obj.rowsToRenderCache;
				this->vbo_vertices = obj.vbo_vertices;
				this->vbo_normals = obj.vbo_normals;
				this->vbo_fowTexCoords = obj.vbo_fowTexCoords;
				this->vbo_surfTexCoords = obj.vbo_surfTexCoords;
				this->vbo_indices = obj.vbo_indices;
				this->indexCount = obj.indexCount;
				this->textureHandle = obj.textureHandle;
				this->texturePath = obj.texturePath;
				this->textureCRC = obj.textureCRC;

				return *this;
			}

			~Layer();
			void load_vbos(bool vboEnabled);
			void render(VisibleQuadContainerCache &qCache);
			void renderVisibleLayer();

			std::vector<Vec3f> vertices, normals;
			std::vector<Vec2f> fowTexCoords, surfTexCoords;
			std::vector<GLuint> indices;
			std::map<Vec2i, int> cellToIndicesMap;
			std::map<Quad2i, vector<pair<int,int> > > rowsToRenderCache;

			GLuint vbo_vertices, vbo_normals,
				vbo_fowTexCoords, vbo_surfTexCoords,
				vbo_indices;
			int indexCount;
			int textureHandle;
			string texturePath;
			uint32 textureCRC;
		};
		typedef std::vector<Layer*> Layers;
		Layers layers;
		Quad2i lastVisibleQuad;
	} mapRenderer;

	bool ExtractFrustum(VisibleQuadContainerCache &quadCacheItem);
	bool PointInFrustum(vector<vector<float> > &frustum, float x, float y, float z );
	bool SphereInFrustum(vector<vector<float> > &frustum,  float x, float y, float z, float radius);
	bool CubeInFrustum(vector<vector<float> > &frustum, float x, float y, float z, float size );

private:
	Renderer();
	~Renderer();

public:
	static Renderer &getInstance();
	static bool isEnded();
	//bool isMasterserverMode() const { return masterserverMode; }

	void addToDeferredParticleSystemList(std::pair<ParticleSystem *, ResourceScope> deferredParticleSystem);
	void manageDeferredParticleSystems();

	void reinitAll();

    //init
	void init();
	void initGame(const Game *game, GameCamera *gameCamera);
	void initMenu(const MainMenu *mm);
	void reset3d();
	void reset2d();
	void reset3dMenu();

	//end
	void end();
	void endScenario();
	void endMenu();
	void endGame(bool isFinalEnd);

	//get
	inline int getTriangleCount() const	{return triangleCount;}
	inline int getPointCount() const		{return pointCount;}

	//misc
	void reloadResources();

	//engine interface
	void initTexture(ResourceScope rs, Texture *texture);
	void endTexture(ResourceScope rs, Texture *texture,bool mustExistInList=false);
	void endLastTexture(ResourceScope rs, bool mustExistInList=false);

	Model *newModel(ResourceScope rs,const string &path,bool deletePixMapAfterLoad=false,std::map<string,vector<pair<string, string> > > *loadedFileList=NULL, string *sourceLoader=NULL);
	void endModel(ResourceScope rs, Model *model, bool mustExistInList=false);
	void endLastModel(ResourceScope rs, bool mustExistInList=false);

	Texture2D *newTexture2D(ResourceScope rs);
	Texture3D *newTexture3D(ResourceScope rs);
	Font2D *newFont(ResourceScope rs);
	Font3D *newFont3D(ResourceScope rs);
	void endFont(::Shared::Graphics::Font *font, ResourceScope rs, bool mustExistInList=false);
	void resetFontManager(ResourceScope rs);

	inline TextRenderer2D *getTextRenderer() const	{return textRenderer;}
	inline TextRenderer3D *getTextRenderer3D() const	{return textRenderer3D;}

	void manageParticleSystem(ParticleSystem *particleSystem, ResourceScope rs);
	void cleanupParticleSystems(vector<ParticleSystem *> &particleSystems,ResourceScope rs);
	void cleanupUnitParticleSystems(vector<UnitParticleSystem *> &particleSystems,ResourceScope rs);
	bool validateParticleSystemStillExists(ParticleSystem * particleSystem,ResourceScope rs) const;
	void removeParticleSystemsForParticleOwner(ParticleOwner * particleOwner,ResourceScope rs);
	void updateParticleManager(ResourceScope rs,int renderFps=-1);
	void renderParticleManager(ResourceScope rs);
	void swapBuffers();

    //lights and camera
	void setupLighting();
	void setupLightingForRotatedModel();
	void loadGameCameraMatrix();
	void loadCameraMatrix(const Camera *camera);
	void computeVisibleQuad();

    //basic rendering
	void renderMouse2d(int mouseX, int mouseY, int anim, float fade= 0.f);
    void renderMouse3d();

    void renderGhostModel(const UnitType *building, const Vec2i pos,CardinalDir facing,Vec4f *forceColor=NULL);

    void renderBackground(const Texture2D *texture);
	void renderTextureQuad(int x, int y, int w, int h, const Texture2D *texture, float alpha=1.f,const Vec3f *color=NULL);
	void renderConsole(const Console *console, const bool showAll=false, const bool showMenuConsole=false, int overrideMaxConsoleLines=-1);
	void renderConsoleLine3D(int lineIndex, int xPosition, int yPosition, int lineHeight, Font3D* font, string stringToHightlight, const ConsoleLineInfo *lineInfo);
	void renderConsoleLine(int lineIndex, int xPosition, int yPosition, int lineHeight, Font2D* font, string stringToHightlight, const ConsoleLineInfo *lineInfo);

	void renderChatManager(const ChatManager *chatManager);
	void renderClock();
	void renderPerformanceStats();
	void renderResourceStatus();
	void renderSelectionQuad();
	void renderText(const string &text, Font2D *font, float alpha, int x, int y, bool centered= false);
	void renderText(const string &text, Font2D *font, const Vec3f &color, int x, int y, bool centered= false);
	void renderText(const string &text, Font2D *font, const Vec4f &color, int x, int y, bool centered=false);
	void renderTextShadow(const string &text, Font2D *font,const Vec4f &color, int x, int y, bool centered= false);

	void renderText3D(const string &text, Font3D *font, float alpha, int x, int y, bool centered);
	void renderText3D(const string &text, Font3D *font, const Vec3f &color, int x, int y, bool centered);
	void renderText3D(const string &text, Font3D *font, const Vec4f &color, int x, int y, bool centered);
	void renderTextShadow3D(const string &text, Font3D *font,const Vec4f &color, int x, int y, bool centered=false);
	void renderProgressBar3D(int size, int x, int y, Font3D *font, int customWidth=-1, string prefixLabel="", bool centeredText=true,int customHeight=-1);

	Vec2f getCentered3DPos(const string &text, Font3D *font, Vec2f &pos, int w, int h, bool centeredW, bool centeredH);
	void renderTextBoundingBox3D(const string &text, Font3D *font, const Vec4f &color, int x, int y, int w, int h, bool centeredW, bool centeredH, bool editModeEnabled, int maxEditWidth, int maxEditRenderWidth);
	void renderTextBoundingBox3D(const string &text, Font3D *font, const Vec3f &color, int x, int y, int w, int h, bool centeredW, bool centeredH, bool editModeEnabled,int maxEditWidth, int maxEditRenderWidth);
	void renderTextBoundingBox3D(const string &text, Font3D *font, float alpha, int x, int y, int w, int h, bool centeredW, bool centeredH, bool editModeEnabled,int maxEditWidth, int maxEditRenderWidth);

	void renderTextSurroundingBox(int x, int y, int w, int h,int maxEditWidth, int maxEditRenderWidth);

	void beginRenderToTexture(Texture2D **renderToTexture);
	void endRenderToTexture(Texture2D **renderToTexture);

	void renderFPSWhenEnabled(int lastFps);

    //components
	void renderLabel(GraphicLabel *label);
	void renderLabel(GraphicLabel *label,const Vec3f *color);
	void renderLabel(GraphicLabel *label,const Vec4f *color);
    void renderButton(GraphicButton *button,const Vec4f *fontColorOverride=NULL,bool *lightedOverride=NULL);
    void renderCheckBox(const GraphicCheckBox *box);
    void renderLine(const GraphicLine *line);
    void renderScrollBar(const GraphicScrollBar *sb);
    void renderListBox(GraphicListBox *listBox);
	void renderMessageBox(GraphicMessageBox *listBox);
	void renderPopupMenu(PopupMenu *menu);

    //complex rendering
    void renderSurface(const int renderFps);
	void renderObjects(const int renderFps);

	void renderWater();
    void renderUnits(bool airUnits, const int renderFps);
    void renderUnitsToBuild(const int renderFps);

	void renderSelectionEffects();
	void renderWaterEffects();
	void renderHud();
	void renderMinimap();
    void renderDisplay();
	void renderMenuBackground(const MenuBackground *menuBackground);
	void renderMapPreview(const MapPreview *map, bool renderAll, int screenX, int screenY,Texture2D **renderToTexture=NULL);
	void renderMenuBackground(Camera *camera, float fade, Model *mainModel, vector<Model *> characterModels,const Vec3f characterPosition, float anim);

	//computing
    bool computePosition(const Vec2i &screenPos, Vec2i &worldPos,bool exactCoords=false);
	void computeSelected(Selection::UnitContainer &units, const Object *&obj, const bool withObjectSelection, const Vec2i &posDown, const Vec2i &posUp);
	void selectUsingColorPicking(Selection::UnitContainer &units, const Object *&obj,const bool withObjectSelection,const Vec2i &posDown, const Vec2i &posUp);
	void selectUsingSelectionBuffer(Selection::UnitContainer &units,const Object *&obj, const bool withObjectSelection,const Vec2i &posDown, const Vec2i &posUp);
	void selectUsingFrustumSelection(Selection::UnitContainer &units,const Object *&obj, const bool withObjectSelection,const Vec2i &posDown, const Vec2i &posUp);


    //gl wrap
	string getGlInfo();
	string getGlMoreInfo();
	void autoConfig();

	//clear
    void clearBuffers();
	void clearZBuffer();

	//shadows
	void renderShadowsToTexture(const int renderFps);

	//misc
	void loadConfig();
	void saveScreen(const string &path,int w=0, int h=0);
	inline Quad2i getVisibleQuad() const		{return visibleQuad;}
	inline Quad2i getVisibleQuadFromCamera() const		{return visibleQuadFromCamera;}
	void renderTeamColorPlane();
	void renderSpecialHighlightUnits(std::map<int,HighlightSpecialUnitInfo> unitHighlightList);
	void renderTeamColorCircle();
	void renderMorphEffects();

	//static
	static Shadows strToShadows(const string &s);
	static string shadowsToStr(Shadows shadows);

	inline const Game * getGame() { return game; }

	void setAllowRenderUnitTitles(bool value);
	inline bool getAllowRenderUnitTitles() { return allowRenderUnitTitles; }
	void renderUnitTitles(Font2D *font, Vec3f color);
	void renderUnitTitles3D(Font3D *font, Vec3f color);
	Vec3f computeScreenPosition(const Vec3f &worldPos);

	void setPhotoMode(bool value) { photoMode = value; }

	inline bool getNo2DMouseRendering() const { return no2DMouseRendering; }
	void setNo2DMouseRendering(bool value) { no2DMouseRendering = value; }

	inline bool getShowDebugUI() const { return showDebugUI; }
	void setShowDebugUI(bool value) { showDebugUI = value; }

	inline int getShowDebugUILevel() const { return showDebugUILevel; }
	void setShowDebugUILevel(int value) { showDebugUILevel=value; }
	void cycleShowDebugUILevel();

	void setLastRenderFps(int value);
	inline int getLastRenderFps() const { return lastRenderFps;}

	VisibleQuadContainerCache & getQuadCache(bool updateOnDirtyFrame=true,bool forceNew=false);
	std::pair<bool,Vec3f> posInCellQuadCache(Vec2i pos);
	Vec3f getMarkedCellScreenPosQuadCache(Vec2i pos);
	void updateMarkedCellScreenPosQuadCache(Vec2i pos);
	void forceQuadCacheUpdate();
	void renderVisibleMarkedCells(bool renderTextHint=false,int x=-1, int y=-1);
	void renderMarkedCellsOnMinimap();

	void renderHighlightedCellsOnMinimap();

	void removeObjectFromQuadCache(const Object *o);
	void removeUnitFromQuadCache(const Unit *unit);

	std::size_t getCurrentPixelByteCount(ResourceScope rs=rsGame) const;
	unsigned int getSaveScreenQueueSize();

	Texture2D *saveScreenToTexture(int x, int y, int width, int height);

	void renderProgressBar(int size, int x, int y, Font2D *font,int customWidth=-1, string prefixLabel="", bool centeredText=true);

	static Texture2D * findTexture(string logoFilename);
	static Texture2D * preloadTexture(string logoFilename);
	inline int getCachedSurfaceDataSize() const { return (int)mapSurfaceData.size(); }

	//void setCustom3dMenuList(GLuint *customlist3dMenu) { this->customlist3dMenu = customlist3dMenu; }
	//inline GLuint * getCustom3dMenuList() const { return this->customlist3dMenu; }
	void setCustom3dMenu(const MainMenu *mm) { this->custom_mm3d = mm; }
	const MainMenu * getCustom3dMenu() { return this->custom_mm3d; }

	void init3dListMenu(const MainMenu *mm);

	void setProgram(Program *program) { this->program = program; }

	void setupRenderForVideo();
	virtual void renderVideoLoading(int progressPercent);

private:
	//private misc
	float computeSunAngle(float time);
	float computeMoonAngle(float time);
	Vec4f computeSunPos(float time);
	Vec4f computeMoonPos(float time);
	inline Vec4f computeWaterColor(float waterLevel, float cellHeight) {
		const float waterFactor= 1.5f;
		return Vec4f(1.f, 1.f, 1.f, clamp((waterLevel-cellHeight) * waterFactor, 0.f, 1.f));
	}
	void checkExtension(const string &extension, const string &msg);

	//selection render
	vector<Object *> renderObjectsFast(bool renderingShadows = false, bool resourceOnly = false, bool colorPickingSelection = false);
	vector<Unit *> renderUnitsFast(bool renderingShadows = false, bool colorPickingSelection = false);

	//gl requirements
	void checkGlCaps();
	void checkGlOptionalCaps();

	//gl init
	void init3dList();
    void init2dList();

	//misc
	void loadProjectionMatrix();
	void enableProjectiveTexturing();

	//private aux drawing
	void renderSelectionCircle(Vec3f v, int size, float radius, float thickness=0.2f);
	void renderTeamColorEffect(Vec3f &v, int heigth, int size, Vec3f color, const Texture2D *texture);
	void renderArrow(const Vec3f &pos1, const Vec3f &pos2, const Vec3f &color, float width);
	void renderTile(const Vec2i &pos);
	void renderQuad(int x, int y, int w, int h, const Texture2D *texture);

	void simpleTask(BaseThread *callingThread,void *userdata);

	//static
    static Texture2D::Filter strToTextureFilter(const string &s);
    void cleanupScreenshotThread();

    void render2dMenuSetup();
    void render3dSetup();
    void render3dMenuSetup(const MainMenu *mm);
};

}} //end namespace

#endif
