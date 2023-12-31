#include <raylib.h>
#include <raymath.h>
#define NO_FONT_AWESOME
#include "rlImGui.h"

#include <imgui.h>
#include <TextEditor.h>

#include <sol/sol.hpp>
#include <functional>
#include <stdarg.h>

class Transform2D {
public:
	Vector2 position{};
	Vector2 scale{};
	float rotation; // In radians
	
	Transform2D(Vector2 position = {0,0}, Vector2 scale = {0,0}, float rotationInRadians = 0)
	{
		this->position = position;
		this->scale = scale;
		this->rotation = rotationInRadians;
	}
	
	Matrix GetTransformMatrix() const {
		Matrix mat = MatrixIdentity();
		mat = MatrixTranslate(position.x, position.y, 0.0f);
		mat = MatrixMultiply(mat, MatrixScale(scale.x, scale.y, 1.0f));
		mat = MatrixMultiply(mat, MatrixRotate({ rotation, 0, 0 }, 1));
		return mat;
	}

	// Helper functions for setting rotation in degrees or radians
	void SetRotationDegrees(float degrees) {
		rotation = DEG2RAD * degrees;
	}

	void SetRotationRadians(float radians) {
		rotation = radians;
	}
	
};



enum UiType
{
	TYPE_NONE = -1,
	BUTTON = 1,
	FRAME = 2,
	TEXTLABEL = 3,
};

#define DECLARE_UI(type) UiType GetClassName() override \
{ return type; } \


// abstract class to represent any ui element
class UiElement
{
public:

	UiElement* parent;
	std::vector<UiElement*> children;

public:
	Transform2D transform;
	Vector2 size; // width & height
	bool isActive;
	Color backgroundColor;
	int backgroundTransparency;
	Color borderColor;
	int borderSizePixel;
	int Transparency;
	bool isVisible;
	int zIndex;
public:
	UiElement() : parent(nullptr), size({0,0}), isActive(true), backgroundColor(WHITE), backgroundTransparency(0),
	              borderColor(BLACK),
	              borderSizePixel(0),
	              Transparency(0), isVisible(true), zIndex(0), transform()
	{
	}

	virtual UiType GetClassName()
	{
		return UiType::TYPE_NONE;
	}

	void AddChild(UiElement* child)
	{
		child->parent = this;
		children.emplace_back(child);
	}
	virtual void Draw() = 0;

};


class Frame : public UiElement
{
	DECLARE_UI(UiType::FRAME);

};

class Button : public UiElement
{
public:
	DECLARE_UI(UiType::BUTTON);

	Button() : UiElement()
	{
		
	}
	
	void Draw() override
	{
		
	}
};

class TextLabel : public UiElement
{
public:
	DECLARE_UI(UiType::TEXTLABEL);
	std::string Text;
	TextLabel(std::string text = "") : UiElement(), Text(text)
	{
		
	}

	void Draw()
	{
		
	}
};

// the engine behind interfaces
class UiEngine
{
public:
	std::vector<UiElement*> m_UiElements;


	UiEngine()
	{

	}

	void AddElement(UiElement* elem)
	{
		m_UiElements.emplace_back(elem);
	}
	
	template<typename... Args>
	void Construct(UiElement* element, Args&&... args)
	{
		switch (element->GetClassName())
		{
		case UiType::BUTTON:
			
			//dynamic_cast<Button*>(element)->create(std::forward<Args>(args)...);
			break;

		case UiType::TYPE_NONE:
			std::cout << "type none" << std::endl;
			break;
		
		}
	}
};

class LuaEngine
{
public:
	sol::state luaState;


	void Init()
	{
		luaState.open_libraries(sol::lib::base, sol::lib::debug, sol::lib::math, sol::lib::table, sol::lib::string, sol::lib::utf8);

	}

	void RunScriptSafe(std::string script)
	{
		luaState.script(script);
	}

	void VarArgFunc(int e, ...)
	{
		va_list lst;
		va_start(lst, e);

	}
};


class LuaEditor
{
public:
	TextEditor editor;
	LuaEngine* luaEngine;

	LuaEditor()
	{
		this->editor = TextEditor();
		luaEngine = nullptr;
	}
	void Init()
	{
		auto lang = TextEditor::LanguageDefinition::Lua();
		editor.SetLanguageDefinition(lang);

		
	}

	void SetLuaEngine(LuaEngine* luaEngine)
	{
		this->luaEngine = luaEngine;
	}

	void Render()
	{

		ImGui::BeginChild("Lua Editor", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
		auto cpos = editor.GetCursorPosition();

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save"))
				{
					auto textToSave = editor.GetText();
					/// save text....
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				bool ro = editor.IsReadOnly();
				if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
					editor.SetReadOnly(ro);
				ImGui::Separator();

				if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
					editor.Undo();
				if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
					editor.Redo();

				ImGui::Separator();

				if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
					editor.Copy();
				if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
					editor.Cut();
				if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
					editor.Delete();
				if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
					editor.Paste();

				ImGui::Separator();

				if (ImGui::MenuItem("Select all", nullptr, nullptr))
					editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Dark palette"))
					editor.SetPalette(TextEditor::GetDarkPalette());
				if (ImGui::MenuItem("Light palette"))
					editor.SetPalette(TextEditor::GetLightPalette());
				if (ImGui::MenuItem("Retro blue palette"))
					editor.SetPalette(TextEditor::GetRetroBluePalette());
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Execute"))
			{
				if (ImGui::MenuItem("Run", "F5"))
					luaEngine->RunScriptSafe(editor.GetText());
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
			editor.IsOverwrite() ? "Ovr" : "Ins",
			editor.CanUndo() ? "*" : " ",
			editor.GetLanguageDefinition().mName.c_str(), "on the fly");
		editor.Render("TextEditor");

		ImGui::EndChild();
	}
};

int main()
{
	InitWindow(800, 600, "Raylib lua funny");
	SetTargetFPS(60);

	rlImGuiSetup(true);
	LuaEditor editor;
	LuaEngine luaEngine;
	luaEngine.Init();
	UiEngine uiEngine;
	Button btn;
	//uiEngine.Construct(&btn, 200, 200, 100, 100, WHITE);
	editor.SetLuaEngine(&luaEngine);
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(WHITE);

		rlImGuiBegin();
		
		ImGui::Begin("Text Editor Demo");
		ImGui::SetWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		if (ImGui::BeginTabBar("Windows"))
		{
			if (ImGui::BeginTabItem("Editor"))
			{
				editor.Render();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("UI"))
			{
				ImGui::Text("Hello");
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
		rlImGuiEnd();
		EndDrawing();
	}

	rlImGuiShutdown();

	CloseWindow();
}