// Original implementation
#include "gepch.h"
#include "UIManager.h"

namespace GameEngine {

	void UIManager::Init()
	{
		myImGuiLayer = MakeOwn<ImGuiLayer>();
		myImGuiLayer->OnAttach();
	}

	void UIManager::Shutdown()
	{
		myImGuiLayer->OnDetach();
	}

	bool UIManager::OnEvent(Event& e)
	{
		myImGuiLayer->OnEvent(e);
		return e.Handled;
	}

	void UIManager::Render(const std::function<void()>& renderFn)
	{
		myImGuiLayer->Begin();
		renderFn();
		myImGuiLayer->End();
	}

}
