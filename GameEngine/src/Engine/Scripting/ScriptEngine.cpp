#include "gepch.h"
#include "ScriptEngine.h"

#include "ScriptGlue.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/threads.h"

#include "FileWatch.h"

#include "Engine/Core/Application.h"
#include "Engine/Core/Timer.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Core/FileSystem.h"

#include "Engine/Project/Project.h"

namespace GameEngine {

	static std::unordered_map<std::string, ScriptFieldType> gsScriptFieldTypeMap =
	{
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },
		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.Char", ScriptFieldType::Char },
		{ "System.Int16", ScriptFieldType::Short },
		{ "System.Int32", ScriptFieldType::Int },
		{ "System.Int64", ScriptFieldType::Long },
		{ "System.Byte", ScriptFieldType::Byte },
		{ "System.UInt16", ScriptFieldType::UShort },
		{ "System.UInt32", ScriptFieldType::UInt },
		{ "System.UInt64", ScriptFieldType::ULong },

		{ "GameEngine.Vector2", ScriptFieldType::Vector2 },
		{ "GameEngine.Vector3", ScriptFieldType::Vector3 },
		{ "GameEngine.Vector4", ScriptFieldType::Vector4 },

		{ "GameEngine.Entity", ScriptFieldType::Entity },
	};

	namespace Utils {

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
		{
			ScopedBuffer fileData = FileSystem::ReadFileBinary(assemblyPath);

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), fileData.Size(), 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				// Log some error message using the errorMessage data
				return nullptr;
			}

			if (loadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");

				if (std::filesystem::exists(pdbPath))
				{
					ScopedBuffer pdbFileData = FileSystem::ReadFileBinary(pdbPath);
					mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), pdbFileData.Size());
					GE_CORE_INFO("Loaded PDB {}", pdbPath);
				}
			}

			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);

			return assembly;
		}

		void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
				GE_CORE_TRACE("{}.{}", nameSpace, name);
			}
		}

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			auto it = gsScriptFieldTypeMap.find(typeName);
			if (it == gsScriptFieldTypeMap.end())
			{
				GE_CORE_ERROR("Unknown type: {}", typeName);
				return ScriptFieldType::None;
			}

			return it->second;
		}

	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::filesystem::path CoreAssemblyFilepath;
		std::filesystem::path AppAssemblyFilepath;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Handle<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Handle<ScriptInstance>> EntityInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		Own<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
		bool AssemblyReloadPending = false;

#ifdef GE_DEBUG
		bool EnableDebugging = true;
#else
		bool EnableDebugging = false;
#endif
		// Runtime

		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* gsData = nullptr;

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event change_type)
	{
		if (!gsData->AssemblyReloadPending && change_type == filewatch::Event::modified)
		{
			gsData->AssemblyReloadPending = true;

			Application::GetInstance().SubmitToMainThread([]()
			{
				gsData->AppAssemblyFileWatcher.reset();
				ScriptEngine::ReloadAssembly();
			});
		}
	}

	void ScriptEngine::Init()
	{
		gsData = new ScriptEngineData();

		InitMono();
		ScriptGlue::RegisterFunctions();

		bool status = LoadAssembly("Resources/Scripts/GameEngine-ScriptCore.dll");
		if (!status)
		{
			GE_CORE_ERROR("[ScriptEngine] Could not load GameEngine-ScriptCore assembly.");
			return;
		}
		
		auto scriptModulePath = Project::GetAssetDirectory() / Project::GetActive()->GetConfig().ScriptModulePath;
		status = LoadAppAssembly(scriptModulePath);
		if (!status)
		{
			GE_CORE_ERROR("[ScriptEngine] Could not load app assembly.");
			return;
		}

		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		// Retrieve and instantiate class
		gsData->EntityClass = ScriptClass("GameEngine", "Entity", true);
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete gsData;
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		if (gsData->EnableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		MonoDomain* rootDomain = mono_jit_init("GameEngineJITRuntime");
		GE_CORE_ASSERT(rootDomain);

		// Store the root domain pointer
		gsData->RootDomain = rootDomain;

		if (gsData->EnableDebugging)
			mono_debug_domain_create(gsData->RootDomain);

		mono_thread_set_main(mono_thread_current());
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(gsData->AppDomain);
		gsData->AppDomain = nullptr;
		
		mono_jit_cleanup(gsData->RootDomain);
		gsData->RootDomain = nullptr;
	}

	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		gsData->AppDomain = mono_domain_create_appdomain("GameEngineScriptRuntime", nullptr);
		mono_domain_set(gsData->AppDomain, true);

		gsData->CoreAssemblyFilepath = filepath;
		gsData->CoreAssembly = Utils::LoadMonoAssembly(filepath, gsData->EnableDebugging);
		if (gsData->CoreAssembly == nullptr)
			return false;

		gsData->CoreAssemblyImage = mono_assembly_get_image(gsData->CoreAssembly);
		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		gsData->AppAssemblyFilepath = filepath;
		gsData->AppAssembly = Utils::LoadMonoAssembly(filepath, gsData->EnableDebugging);
		if (gsData->AppAssembly == nullptr)
			return false;

		gsData->AppAssemblyImage = mono_assembly_get_image(gsData->AppAssembly);

		gsData->AppAssemblyFileWatcher = MakeOwn<filewatch::FileWatch<std::string>>(filepath.string(), OnAppAssemblyFileSystemEvent);
		gsData->AssemblyReloadPending = false;
		return true;
	}

	void ScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(gsData->AppDomain);

		LoadAssembly(gsData->CoreAssemblyFilepath);
		LoadAppAssembly(gsData->AppAssemblyFilepath);
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		// Retrieve and instantiate class
		gsData->EntityClass = ScriptClass("GameEngine", "Entity", true);
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		gsData->SceneContext = scene;
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return gsData->EntityClasses.find(fullClassName) != gsData->EntityClasses.end();
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (ScriptEngine::EntityClassExists(sc.ClassName))
		{
			UUID entityID = entity.GetUUID();

			Handle<ScriptInstance> instance = MakeHandle<ScriptInstance>(gsData->EntityClasses[sc.ClassName], entity);
			gsData->EntityInstances[entityID] = instance;

			// Copy field values
			if (gsData->EntityScriptFields.find(entityID) != gsData->EntityScriptFields.end())
			{
				const ScriptFieldMap& fieldMap = gsData->EntityScriptFields.at(entityID);
				for (const auto& [name, fieldInstance] : fieldMap)
					instance->SetFieldValueInternal(name, fieldInstance.myBuffer);
			}

			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		if (gsData->EntityInstances.find(entityUUID) != gsData->EntityInstances.end())
		{
			Handle<ScriptInstance> instance = gsData->EntityInstances[entityUUID];
			instance->InvokeOnUpdate((float)ts);
		}
		else
		{
			GE_CORE_ERROR("Could not find ScriptInstance for entity {}",  entityUUID);
		}
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return gsData->SceneContext;
	}

	Handle<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{
		auto it = gsData->EntityInstances.find(entityID);
		if (it == gsData->EntityInstances.end())
			return nullptr;

		return it->second;
	}


	Handle<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{
		if (gsData->EntityClasses.find(name) == gsData->EntityClasses.end())
			return nullptr;

		return gsData->EntityClasses.at(name);
	}

	void ScriptEngine::OnRuntimeStop()
	{
		gsData->SceneContext = nullptr;

		gsData->EntityInstances.clear();
	}

	std::unordered_map<std::string, Handle<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return gsData->EntityClasses;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		GE_CORE_ASSERT(entity);

		UUID entityID = entity.GetUUID();
		return gsData->EntityScriptFields[entityID];
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		gsData->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(gsData->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(gsData->CoreAssemblyImage, "GameEngine", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(gsData->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(gsData->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, className);
			else
				fullName = className;

			MonoClass* monoClass = mono_class_from_name(gsData->AppAssemblyImage, nameSpace, className);

			if (monoClass == entityClass)
				continue;

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (!isEntity)
				continue;

			Handle<ScriptClass> scriptClass = MakeHandle<ScriptClass>(nameSpace, className);
			gsData->EntityClasses[fullName] = scriptClass;


			// This routine is an iterator routine for retrieving the fields in a class.
			// You must pass a gpointer that points to zero and is treated as an opaque handle
			// to iterate over all of the elements. When no more values are available, the return value is NULL.

			int fieldCount = mono_class_num_fields(monoClass);
			GE_CORE_WARN("{} has {} fields:", className, fieldCount);
			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				if (flags & FIELD_ATTRIBUTE_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
					GE_CORE_WARN("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

					scriptClass->myFields[fieldName] = { fieldType, fieldName, field };
				}
			}

		}

		auto& entityClasses = gsData->EntityClasses;

		//mono_field_get_value()

	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return gsData->CoreAssemblyImage;
	}


	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		GE_CORE_ASSERT(gsData->EntityInstances.find(uuid) != gsData->EntityInstances.end());
		return gsData->EntityInstances.at(uuid)->GetManagedObject();
	}

	MonoString* ScriptEngine::CreateString(const char* string)
	{
		return mono_string_new(gsData->AppDomain, string);
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(gsData->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: myClassNamespace(classNamespace), myClassName(className)
	{
		myMonoClass = mono_class_from_name(isCore ? gsData->CoreAssemblyImage : gsData->AppAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(myMonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(myMonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		return mono_runtime_invoke(method, instance, params, &exception);
	}

	ScriptInstance::ScriptInstance(Handle<ScriptClass> scriptClass, Entity entity)
		: myScriptClass(scriptClass)
	{
		myInstance = scriptClass->Instantiate();

		myConstructor = gsData->EntityClass.GetMethod(".ctor", 1);
		myOnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		myOnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		// Call Entity constructor
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			myScriptClass->InvokeMethod(myInstance, myConstructor, &param);
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (myOnCreateMethod)
			myScriptClass->InvokeMethod(myInstance, myOnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		if (myOnUpdateMethod)
		{
			void* param = &ts;
			myScriptClass->InvokeMethod(myInstance, myOnUpdateMethod, &param);
		}
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer)
	{
		const auto& fields = myScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_get_value(myInstance, field.ClassField, buffer);
		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = myScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_set_value(myInstance, field.ClassField, (void*)value);
		return true;
	}

}
