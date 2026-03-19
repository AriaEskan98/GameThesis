// Taken from Hazel Engine by TheCherno
// Source: https://github.com/TheCherno/Hazel
// Changes: None
namespace GameEngine
{
	public class Input
	{
		public static bool IsKeyDown(KeyCode keycode)
		{
			return InternalCalls.Input_IsKeyDown(keycode);
		}
	}
}
