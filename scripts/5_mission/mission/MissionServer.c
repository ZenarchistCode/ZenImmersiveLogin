modded class MissionServer
{
	// Whether or not to disable creating a fireplace upon spawn
	private bool m_DisableFireSpawn = false;

	// Check if server has disabled spawn fire
	override void OnInit()
	{
		super.OnInit();

		// Check if disablespawnfire.txt exists in my server config folder
		m_DisableFireSpawn = FileExist("$profile:\\Zenarchist\\disablespawnfire.txt");
	};

	// New player spawn - create a fire nearby if it's not disabled
	override void EquipCharacter(MenuDefaultCharacterData char_data)
	{
		super.EquipCharacter(char_data);

		if (!m_DisableFireSpawn)
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CreateFire, 10, false, m_player);
	}

	// Create a fireplace near the player
	void CreateFire(PlayerBase player)
	{
		if (!player)
			return;

		vector posXZ = player.GetPosition();
		float randX = Math.RandomFloatInclusive(2.0, 2.5);
		float randZ = Math.RandomFloatInclusive(2.0, 2.5);

		// Create fire at a safe distance and random location within 2.5m of player
		if (Math.RandomIntInclusive(1, 2) == 1)
			randX = randX * -1;

		if (Math.RandomIntInclusive(1, 2) == 1)
			randZ = randZ * -1;

		// Get fire surface pos
		float x = posXZ[0] + randX;
		float z = posXZ[2] + randZ;
		float y = GetGame().SurfaceY(x, z);
		vector firePos = { x,y,z };

		// Check surface type
		string surface_type;
		GetGame().SurfaceGetType(firePos[0], firePos[2], surface_type);
		surface_type.ToLower();

		// Don't start fires under water
		if (surface_type.Contains("water"))
			return;

		// Create fire
		FireplaceBase fire = FireplaceBase.Cast(GetGame().CreateObject("Fireplace", firePos));

		if (!fire)
			return;

		// If the fire is created under a roof, just delete it
		if (MiscGameplayFunctions.IsUnderRoof(fire))
		{
			fire.DeleteSafe();
			return;
		}

		// All checks passed - create fire and ignite it
		fire.PlaceOnSurface();
		fire.Synchronize();

		// Create tinder & fuel
		ItemBase wood = ItemBase.Cast(fire.GetInventory().CreateAttachment("Firewood"));
		ItemBase stick = ItemBase.Cast(fire.GetInventory().CreateAttachment("WoodenStick"));
		ItemBase rag = ItemBase.Cast(fire.GetInventory().CreateAttachment("Rag"));

		// Lock tinder & fuel
		if (wood)
			wood.LockToParent();

		if (stick)
			stick.LockToParent();
		
		if (rag)
			rag.LockToParent();

		// Ignite fire
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(StartFire, 100, false, fire);
	}

	// Ignite the fire
	void StartFire(FireplaceBase fire)
	{
		if (fire)
		{
			// Ignite fire
			fire.StartFire(true);
			// Put fire out after 12 seconds
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ExtinguishFire, 12000, false, fire);
		}
	}

	// Extinguish the fire so the player cannot suicide on it
	void ExtinguishFire(FireplaceBase fire)
	{
		if (fire)
		{
			// Set fire extinguish sound effect
			fire.SetExtinguishingState();
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(StopFire, 3000, false, fire);
		}
	}

	// Stop the fire and set lifetime short (1 minute) so that it gets deleted when player leaves area
	void StopFire(FireplaceBase fire)
	{
		if (fire)
		{
			// Stop fire burning
			fire.StopFire();
			fire.SetLifetime(60);
			int attachments_count = fire.GetInventory().AttachmentCount();

			// Delete all attachments so the player cannot re-ignite the fire
			for (int i = 0; i < attachments_count; i++)
			{
				ItemBase item = ItemBase.Cast(fire.GetInventory().GetAttachmentFromIndex(i));

				if (item)
					item.DeleteSafe();
			}
		}
	}
};