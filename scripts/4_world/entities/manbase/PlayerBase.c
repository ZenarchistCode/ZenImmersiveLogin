enum ZenSpawnRPCs { OPEN_EYES = 42069159; }

modded class PlayerBase
{
	// How long the eye-opening effect takes during login
	private static const float DEFAULT_SPAWNFX_DURATION = 500;
	private float m_SpawnDarkeningCurrentTime = DEFAULT_SPAWNFX_DURATION;
	// Whether or not to set player invisible during login
	private bool m_ImmersiveLoginInvisible = true;
	private bool m_PlayerHasLoggedIn = false;

	// Reset and register login sync flag
	override void Init()
	{
		super.Init();

		// This is used to make sure we only set players invisible if they recently logged in
		m_PlayerHasLoggedIn = false;
		RegisterNetSyncVariableBool("m_PlayerHasLoggedIn");
	};

	// When player loads, trigger lie-down emote, shut their eyes and set them invisible until lie-down animation is finished
	override void OnPlayerLoaded()
	{
		super.OnPlayerLoaded();

		#ifndef SERVER
		// (Client-side) Simulate "waking up" (only if player is connected to server - otherwise the loading screen player model does some strange shit)
		if (GetGame().IsMultiplayer() && !m_PlayerHasLoggedIn)
		{
			if (IsControlledPlayer())
				ShutEyesClient();

			SetInvisible();
		}
		#else
		// (Server-side) Set player status to 'logged in' so they don't turn invisible when loaded anymore
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ServerSyncLoginInvisibility, 4200, false);
		#endif
	}

	// Called ~4 secs after login. Flags this player object as 'logged in' so do not set them invisible anymore when loaded on new clients
	private void ServerSyncLoginInvisibility()
	{
		m_PlayerHasLoggedIn = true;
		SetSynchDirty();
	}

	// Set character model invisible immediately on connection so we don't see them spawn in standing and then lie down
	private void SetInvisible()
	{
		m_ImmersiveLoginInvisible = true;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SetNotInvisible, 4200, false);
	};

	// Check if the player model that just loaded is not lying down, and if so, set them visible much sooner
	private void CheckNotLyingDown()
	{
		if (!m_EmoteManager || !m_EmoteManager.m_bEmoteIsPlaying || !m_EmoteManager.m_Callback || m_EmoteManager.m_Callback.m_callbackID != DayZPlayerConstants.CMD_GESTUREFB_LYINGDOWN)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(SetNotInvisible);
			SetNotInvisible();
		}
	}

	// After the player is lying down, set them to visible
	private void SetNotInvisible()
	{
		m_ImmersiveLoginInvisible = false;
		SetFlags(EntityFlags.VISIBLE | EntityFlags.SOLID | EntityFlags.ACTIVE, true);
		dBodySetInteractionLayer(this, PhxInteractionLayers.CHARACTER);

		if (!IsControlledPlayer() && GetGame().IsMultiplayer())
		{
			dBodySetInteractionLayer(this, PhxInteractionLayers.NOCOLLISION);
			Update();
			DisableSimulation(false);
		}

		SetInvisible(false);
	};

	// Set logging-in players invisible until they are lying down
	override void EOnPostFrame(IEntity other, int extra)
	{
		super.EOnPostFrame(other, extra);

		if (!GetGame().IsClient())
			return;

		// If player is performing lie-down animation after login, set them as invisible (looks silly to nearby players if they login standing up and then lie down)
		if (!m_PlayerHasLoggedIn && m_ImmersiveLoginInvisible)
		{
			if (!IsControlledPlayer())
			{
				ClearFlags(EntityFlags.VISIBLE | EntityFlags.SOLID | EntityFlags.ACTIVE, true);
				SetScale(0);
				dBodySetInteractionLayer(this, PhxInteractionLayers.NOCOLLISION);
				DisableSimulation(true);
			}

			SetInvisible(true);
		}
	}

	// Client-side - begin immersive login effects
	private void ShutEyesClient()
	{
		// Stop player movement
		GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
		GetGame().GetMission().GetHud().ShowQuickbarUI(false);

		// Lie down
		if (GetEmoteManager() && GetEmoteManager().CanPlayEmote(EmoteConstants.ID_EMOTE_LYINGDOWN))
			GetEmoteManager().CreateEmoteCBFromMenu(EmoteConstants.ID_EMOTE_LYINGDOWN);

		// Blackout screen
		PPERequester_ZenSpawnEffects ppeEffect = PPERequesterBank.GetRequester(PPERequester_ZenSpawnEffects);
		ppeEffect.SetEffectValues(200);

		// Mute volume
		GetGame().GetSoundScene().SetSoundVolume(0, 0);

		// Schedule "wake up" 6 seconds after login
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(StartOpeningEyesClient, 6000, false);
	}

	// Trigger eye-open fx and enable sound
	private void StartOpeningEyesClient()
	{
		// Enable controls, start opening eyes fx, and slowly fade-in volume
		GetGame().GetMission().PlayerControlEnable(true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateSpawnDarknessLevel, 1, false);
		GetGame().GetSoundScene().SetSoundVolume(g_Game.m_volume_sound, 5);
	}

	// Simulate Spawn visual effect
	private void UpdateSpawnDarknessLevel()
	{
		// Calculate vignette percentage
		float percentage = Math.Lerp(0, 2, 1 - ((DEFAULT_SPAWNFX_DURATION - m_SpawnDarkeningCurrentTime) / DEFAULT_SPAWNFX_DURATION));

		// Decrease fx timer
		m_SpawnDarkeningCurrentTime -= 1;
		
		// Update fx values
		PPERequester_ZenSpawnEffects ppeEffect = PPERequesterBank.GetRequester(PPERequester_ZenSpawnEffects);
		ppeEffect.SetEffectValues(percentage);

		// If animation of opening eyes is over, stop updating.
		if (percentage <= 0)
		{
			// If our eyes are no longer shut, remove the Spawn requester effect altogether
			if (ppeEffect)
			{
				ppeEffect.SetEffectValues(0);
				ppeEffect.Stop();
			}

			// Stop update loop
			return;
		}

		// Re-schedule update loop
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateSpawnDarknessLevel, 1, false);
	}
};
