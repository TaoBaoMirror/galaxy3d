#include "GamePlant.hpp"

class LauncherPlant : public Component, public IScreenResizeEventListener
{
protected:
	float pixel_per_unit = 100;
	TextRenderer *m_fps;
	Camera *m_cam;
	UICanvas *m_canvas;
	SpriteBatchRenderer *m_batch_ui;

	std::shared_ptr<TextRenderer> CreateLabel(GameObject *parent, Vector3 &pos, int font_size, LabelPivot::Enum pivot, int order)
	{
		auto label = Label::Create("", "heiti", font_size, pivot, LabelAlign::Auto, true);
		auto tr = GameObject::Create("")->AddComponent<TextRenderer>();
		tr->SetLabel(label);
		tr->SetSortingOrder(order, 0);
		tr->GetTransform()->SetParent(parent->GetTransform());
		tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		tr->GetTransform()->SetLocalPosition(pos);

		return tr;
	}

	virtual void OnScreenResize(int width, int height)
	{
		m_cam->SetOrthographicSize(1 / pixel_per_unit * Screen::GetHeight() / 2);
		m_canvas->SetSize(width, height);
		m_canvas->AnchorTransform(m_fps->GetTransform(), *m_fps->GetAnchor());
		m_batch_ui->GetTransform()->SetLocalPosition(Vector3(0, Mathf::Round(- Screen::GetHeight() / 2.0f), 0));

		auto tp = g_batch_game->GetGameObject()->GetComponent<TweenPosition>();
		if(tp)
		{
			Component::Destroy(std::dynamic_pointer_cast<Component>(tp));
		}
		if(g_ui_bag_up)
		{
			g_batch_game->GetTransform()->SetLocalPosition(Vector3(0, Mathf::Round(1080 / 2.0f - Screen::GetHeight() / 2.0f), 0));
		}
		else
		{
			g_batch_game->GetTransform()->SetLocalPosition(Vector3(0, Mathf::Round(1080 / 2.0f - Screen::GetHeight() / 2.0f) + 290, 0));
		}
	}
	
	virtual void CreateUI(UICanvas *canvas, UIAtlas *atlas)
	{
		auto batch_ui = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
		batch_ui->GetTransform()->SetParent(canvas->GetTransform());
		batch_ui->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		batch_ui->SetSortingOrder(3, 0);
		m_batch_ui = batch_ui.get();

		auto batch_bag = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
		batch_bag->GetTransform()->SetParent(batch_ui->GetTransform());
		batch_bag->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		batch_bag->SetSortingOrder(1, 0);

		auto sprite = atlas->CreateSprite(
			"bar_bottom",
			Vector2(0.5f, 1),
			pixel_per_unit,
			Sprite::Type::Tiled,
			Vector2(1920, 89));
		auto node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(batch_ui->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(4);
		batch_ui->AddSprite(node);

		auto bag = GameObject::Create("bag");
		bag->GetTransform()->SetParent(batch_ui->GetTransform());
		bag->GetTransform()->SetLocalPosition(Vector3(0, 88, 0));
		bag->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		g_bag = bag.get();

		sprite = atlas->CreateSprite(
			"bag",
			Vector2(0.5f, 0),
			pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		node = GameObject::Create("bag")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(bag->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(1);
		batch_bag->AddSprite(node);
		auto collider = node->GetGameObject()->AddComponent<BoxCollider>();
		collider->SetSize(Vector3(1700, 346, 0));
		collider->SetCenter(Vector3(0, -173, 0));

		sprite = atlas->CreateSprite(
			"up",
			Vector2(0.5f, 0.5f),
			pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		node = GameObject::Create("up")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(bag->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(740, 30, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(0);
		batch_bag->AddSprite(node);
		collider = node->GetGameObject()->AddComponent<BoxCollider>();
		collider->SetSize(Vector3(148, 73, 0));
		node->GetGameObject()->AddComponent<ButtonUpEventListener>();

		auto tabs = GameObject::Create("tabs");
		tabs->GetTransform()->SetParent(bag->GetTransform());
		tabs->GetTransform()->SetLocalPosition(Vector3(-740, -7, 0));
		tabs->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

		sprite = atlas->CreateSprite(
			"tab_0_selected",
			Vector2(0.5f, 1),
			pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		node = GameObject::Create("tab_0")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(tabs->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, -75, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(0);
		batch_bag->AddSprite(node);
		collider = node->GetGameObject()->AddComponent<BoxCollider>();
		collider->SetSize(Vector3(147, 73, 0));
		collider->SetCenter(Vector3(0, 37, 0));
		node->GetGameObject()->AddComponent<TabEventListener>();

		auto cards_0 = GameObject::Create("cards_0");
		cards_0->GetTransform()->SetParent(bag->GetTransform());
		cards_0->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		cards_0->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

		auto sprite_card = atlas->CreateSprite(
			"card",
			Vector2(0.5f, 0.5f),
			pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		for(int i=0; i<25; i++)
		{
			auto &item = g_cards[i];
			
			if(!item.name.empty())
			{
				int index = i % 5;
				auto card = GameObject::Create(GTString::ToString(i).str)->AddComponent<SpriteNode>();
				card->GetTransform()->SetParent(tabs->GetTransform());
				card->GetTransform()->SetLocalPosition(Vector3(200 + 270.0f * index, -242, 0));
				card->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
				card->SetSprite(sprite_card);
				card->SetSortingOrder(2);
				batch_bag->AddSprite(card);
				collider = card->GetGameObject()->AddComponent<BoxCollider>();
				collider->SetSize(Vector3(234, 321, 0));
				card->GetGameObject()->AddComponent<CardEventListener>();

				sprite = atlas->CreateSprite(
					item.name + " icon",
					Vector2(0.5f, 1),
					pixel_per_unit,
					Sprite::Type::Simple,
					Vector2(0, 0));
				node = GameObject::Create(GTString::ToString(i).str)->AddComponent<SpriteNode>();
				node->GetTransform()->SetParent(card->GetTransform());
				node->GetTransform()->SetLocalPosition(Vector3(0, -67, 0));
				node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
				node->SetSprite(sprite);
				node->SetSortingOrder(3);
				batch_bag->AddSprite(node);

				auto label_price = CreateLabel(card->GetGameObject().get(), Vector3(-30, -115, 0), 40, LabelPivot::Left, 2);
				label_price->GetLabel()->SetText("<outline>" + GTString::ToString(item.price_base).str + "</outline>");
				label_price->SetColor(Color(228, 255, 0, 255) / 255.0f);

				auto label_count = CreateLabel(card->GetGameObject().get(), Vector3(40, 126, 0), 40, LabelPivot::Left, 2);
				label_count->GetLabel()->SetText("<outline>" + GTString::ToString(item.planted).str + "</outline>");
				label_count->SetColor(Color(202, 237, 255, 255) / 255.0f);

				if(i / 5 != g_tab_current)
				{
					node->GetGameObject()->SetActive(false);
				}
			}
		}

		m_batch_ui->GetTransform()->SetLocalPosition(Vector3(0, Mathf::Round(- Screen::GetHeight() / 2.0f), 0));
	}

    virtual void Start()
	{
		Screen::AddResizeListener(std::dynamic_pointer_cast<IScreenResizeEventListener>(GetComponentPtr()));

		GTUIManager::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");

		auto cam = GameObject::Create("")->AddComponent<Camera>();
		cam->SetOrthographic(true);
		cam->SetOrthographicSize(1 / pixel_per_unit * Screen::GetHeight() / 2);
		cam->SetClipPlane(-1, 1);
		cam->SetCullingMask(LayerMask::GetMask(Layer::UI));
		cam->SetDepth(0);
		cam->SetClearColor(Color(142, 239, 255, 255) / 255.0f);
		m_cam = cam.get();

		auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
		canvas->GetTransform()->SetParent(cam->GetTransform());
		canvas->GetTransform()->SetScale(Vector3(1, 1, 1) * (1.0f / pixel_per_unit));
		m_canvas = canvas.get();

		auto label = Label::Create("", "heiti", 20, LabelPivot::Top, LabelAlign::Auto, true);
		auto tr = GameObject::Create("fps")->AddComponent<TextRenderer>();
		tr->SetLabel(label);
		tr->SetSortingOrder(1000, 0);
		tr->GetTransform()->SetParent(canvas->GetTransform());
		tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		tr->SetColor(Color(0, 0, 0, 1));
		tr->SetAnchor(Vector4(0.5f, 0, 0, 0));
		m_fps = tr.get();
		
		auto batch = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
		batch->GetTransform()->SetParent(canvas->GetTransform());
		batch->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		batch->SetSortingOrder(0, 0);
		g_batch_game = batch.get();

		auto atlas = GTUIManager::LoadAtlas("plant", Application::GetDataPath() + "/Assets/image/plant.json");

		g_grass_node_0 = GameObject::Create("grass_node_0").get();
		g_grass_node_0->GetTransform()->SetParent(batch->GetTransform());
		g_grass_node_0->GetTransform()->SetLocalPosition(Vector3(0, -325, 0));
		g_grass_node_0->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

		g_grass_node_0_copy = GameObject::Create("grass_node_0").get();
		g_grass_node_0_copy->GetTransform()->SetParent(batch->GetTransform());
		g_grass_node_0_copy->GetTransform()->SetLocalPosition(Vector3(0, -325, 0));
		g_grass_node_0_copy->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		g_grass_node_0_copy->SetActive(false);

		g_grass_node_1 = GameObject::Create("grass_node_1").get();
		g_grass_node_1->GetTransform()->SetParent(batch->GetTransform());
		g_grass_node_1->GetTransform()->SetLocalPosition(Vector3(0, -385, 0));
		g_grass_node_1->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

		g_grass_node_1_copy = GameObject::Create("grass_node_1").get();
		g_grass_node_1_copy->GetTransform()->SetParent(batch->GetTransform());
		g_grass_node_1_copy->GetTransform()->SetLocalPosition(Vector3(0, -385, 0));
		g_grass_node_1_copy->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		g_grass_node_1_copy->SetActive(false);

		auto grass_0 = atlas->CreateSprite(
			"grass_0",
			Vector2(1, 1),
			pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(1025, 120),
			Vector4(1, 0, -1, 0));
		auto node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_0->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_0);
		node->SetSortingOrder(1);
		batch->AddSprite(node);

		auto grass_1 = atlas->CreateSprite(
			"grass_1",
			Vector2(0, 1),
			pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(1025, 120),
			Vector4(1, 0, -1, 0));
		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_0->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_1);
		node->SetSortingOrder(1);
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_1->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_0);
		node->SetSortingOrder(2);
		node->SetColor(Color(0.5f, 0.8f, 0.5f, 1));
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_1->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_1);
		node->SetSortingOrder(2);
		node->SetColor(Color(0.5f, 0.8f, 0.5f, 1));
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_0_copy->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_0);
		node->SetSortingOrder(1);
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_0_copy->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_1);
		node->SetSortingOrder(1);
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_1_copy->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_0);
		node->SetSortingOrder(2);
		node->SetColor(Color(0.5f, 0.8f, 0.5f, 1));
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_1_copy->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_1);
		node->SetSortingOrder(2);
		node->SetColor(Color(0.5f, 0.8f, 0.5f, 1));
		batch->AddSprite(node);

		auto sprite_white = atlas->CreateSprite(
			"white",
			Vector2(0.5f, 0),
			pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(1920, 500),
			Vector4(1, 1, -1, -1));

		// transparent sprite for ground boxcollider
		auto ground = GameObject::Create("ground")->AddComponent<SpriteNode>();
		ground->GetTransform()->SetParent(batch->GetTransform());
		ground->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		ground->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		ground->SetSprite(sprite_white);
		ground->SetSortingOrder(0);
		ground->SetColor(Color(1, 1, 1, 0));
		batch->AddSprite(ground);
		auto collider_ground = ground->GetGameObject()->AddComponent<BoxCollider>();
		collider_ground->SetSize(Vector3(1920, 1920, 0));
		ground->GetGameObject()->AddComponent<GroundEventListener>();

		auto sprite = atlas->CreateSprite(
			"ground",
			Vector2(0.5f, 0),
			pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(ground->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, -325, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(3);
		batch->AddSprite(node);
		g_ground_node = node->GetGameObject().get();

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(ground->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, -325, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(3);
		batch->AddSprite(node);
		g_ground_node_copy = node->GetGameObject().get();
		g_ground_node_copy->SetActive(false);

		sprite = atlas->CreateSprite(
			"wave",
			Vector2(0, 1),
			pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(199, 50),
			Vector4(1, 0, -1, -2));

		auto wave_node_0 = GameObject::Create("");
		wave_node_0->GetTransform()->SetParent(batch->GetTransform());
		wave_node_0->GetTransform()->SetLocalPosition(Vector3(0, -410, 0));
		wave_node_0->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		auto tp = wave_node_0->AddComponent<TweenPosition>();
		tp->duration = 3.0f;
		tp->loop = true;
		tp->curve = AnimationCurve();
		tp->curve.keys.push_back(Keyframe(0, 0, 1, 1));
		tp->curve.keys.push_back(Keyframe(0.5f, 1, 1, -1));
		tp->curve.keys.push_back(Keyframe(1, 0, -1, -1));
		tp->from = Vector3(0, -410, 0);
		tp->to = Vector3(0, -390, 0);
		g_wave_node_0 = wave_node_0.get();
		
		for(int i=0; i<11; i++)
		{
			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(g_wave_node_0->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(-1920 / 2.0f + (i - 1) * 199.0f, 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 0.7f, 1));
			node->SetSprite(sprite);
			node->SetSortingOrder(5);
			batch->AddSprite(node);

			auto ts = node->GetGameObject()->AddComponent<TweenScale>();
			ts->duration = 3.0f;
			ts->loop = true;
			ts->curve = AnimationCurve();
			ts->curve.keys.push_back(Keyframe(0, 0, 1, 1));
			ts->curve.keys.push_back(Keyframe(0.5f, 1, 1, -1));
			ts->curve.keys.push_back(Keyframe(1, 0, -1, -1));
			ts->from = Vector3(1, 0.35f, 1);
			ts->to = Vector3(1, 0.75f, 1);
		}

		auto wave_node_1 = GameObject::Create("");
		wave_node_1->GetTransform()->SetParent(batch->GetTransform());
		wave_node_1->GetTransform()->SetLocalPosition(Vector3(0, -440, 0));
		wave_node_1->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		tp = wave_node_1->AddComponent<TweenPosition>();
		tp->duration = 3.0f;
		tp->loop = true;
		tp->curve = AnimationCurve();
		tp->curve.keys.push_back(Keyframe(0, 0, 1, 1));
		tp->curve.keys.push_back(Keyframe(0.5f, 1, 1, -1));
		tp->curve.keys.push_back(Keyframe(1, 0, -1, -1));
		tp->from = Vector3(0, -440, 0);
		tp->to = Vector3(0, -420, 0);
		g_wave_node_1 = wave_node_1.get();
		
		for(int i=0; i<11; i++)
		{
			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(g_wave_node_1->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(-1920 / 2.0f + (i - 1) * 199.0f, 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 0.7f, 1));
			node->SetSprite(sprite);
			node->SetSortingOrder(5);
			batch->AddSprite(node);

			auto ts = node->GetGameObject()->AddComponent<TweenScale>();
			ts->duration = 3.0f;
			ts->loop = true;
			ts->curve = AnimationCurve();
			ts->curve.keys.push_back(Keyframe(0, 0, 1, 1));
			ts->curve.keys.push_back(Keyframe(0.5f, 1, 1, -1));
			ts->curve.keys.push_back(Keyframe(1, 0, -1, -1));
			ts->from = Vector3(1, 0.35f, 1);
			ts->to = Vector3(1, 0.75f, 1);
		}

		// for water
		float water_offset = 2;
		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(batch->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, -410 + water_offset, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite_white);
		node->SetSortingOrder(4);
		node->SetColor(Color(57, 131, 254, 255) / 255.0f);
		batch->AddSprite(node);
		tp = node->GetGameObject()->AddComponent<TweenPosition>();
		tp->duration = 3.0f;
		tp->loop = true;
		tp->curve = AnimationCurve();
		tp->curve.keys.push_back(Keyframe(0, 0, 1, 1));
		tp->curve.keys.push_back(Keyframe(0.5f, 1, 1, -1));
		tp->curve.keys.push_back(Keyframe(1, 0, -1, -1));
		tp->from = Vector3(0, -410 + water_offset, 0);
		tp->to = Vector3(0, -390 + water_offset, 0);
		
		g_batch_game->GetTransform()->SetLocalPosition(Vector3(0, Mathf::Round(1080 / 2.0f - Screen::GetHeight() / 2.0f), 0));

		CreateUI(canvas.get(), atlas.get());

		cam->GetGameObject()->SetLayerRecursively(Layer::UI);
		cam->GetTransform()->SetParent(GetTransform());

		g_map_pos = 0;
		g_grass_pos_0 = 0;
		set_grass_pos((int) g_grass_pos_0, g_grass_node_0, g_grass_node_0_copy);
		g_grass_pos_1 = 1025;
		set_grass_pos((int) g_grass_pos_1, g_grass_node_1, g_grass_node_1_copy);
		g_ground_pos = 0;
		set_ground_pos((int) g_ground_pos, g_ground_node, g_ground_node_copy);
		g_wave_pos_0 = 0;
		set_wave_pos((int) g_wave_pos_0, g_wave_node_0);
		g_wave_pos_1 = 100;
		set_wave_pos((int) g_wave_pos_1, g_wave_node_1);

		g_grass_pos_0_init = g_grass_pos_0;
		g_grass_pos_1_init = g_grass_pos_1;
		g_ground_pos_init = g_ground_pos;
		g_wave_pos_0_init = g_wave_pos_0;
		g_wave_pos_1_init = g_wave_pos_1;
	}

	virtual void Update()
	{
		std::string hit_name;
		auto hit = UICanvas::GetRayHitObject().lock();
		if(hit)
		{
			hit_name = hit->GetName();
		}

		m_fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()).str + "\n" +
			"draw call:" + GTString::ToString(GTTime::GetDrawCall()).str + "\n" +
			"ray hit ui:" + hit_name + "\n"
			"map_x:" + GTString::ToString((int) g_map_pos).str);

		float wave_speed = GTTime::GetDeltaTime() * 120;
		
		g_wave_pos_0 -= wave_speed;
		g_wave_pos_0_init -= wave_speed;
		set_wave_pos((int) g_wave_pos_0, g_wave_node_0);

		g_wave_pos_1 -= wave_speed * 2.0f;
		g_wave_pos_1_init -= wave_speed * 2.0f;
		set_wave_pos((int) g_wave_pos_1, g_wave_node_1);
	}
};