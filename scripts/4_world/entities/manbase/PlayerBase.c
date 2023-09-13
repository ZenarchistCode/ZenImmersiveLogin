modded class PlayerBase
{
	// Visual FX variables
	private static const float DEFAULT_SPAWNFX_DURATION = 500;
	private float m_IL_m_SpawnDarkeningCurrentTime;
	private ref PPERequester_ZenSpawnEffects m_IL_ppeEffect;

	// Login sync variables
	private bool m_ZenLoginHasFinishedClient;
	private bool m_ZenLoginHasFinishedServer;
	private bool m_ZenLoginForceCancel;

	void PlayerBase()
	{
		RegisterNetSyncVariableBool("m_ZenLoginHasFinishedServer");
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if (m_ZenLoginHasFinishedClient == m_ZenLoginHasFinishedServer)
			return;

		m_ZenLoginHasFinishedClient = m_ZenLoginHasFinishedServer;
		if (m_ZenLoginHasFinishedClient && !IsFlagSet(EntityFlags.VISIBLE))
		{
			// Player who is not us has sync'd as logged in, set them visible
			if (!IsControlledPlayer())
			{
				SetFlags(EntityFlags.VISIBLE, true);
				Update();
			}
		}
	}

	override void OnPlayerLoaded()
	{
		super.OnPlayerLoaded();

		if (GetGame().IsClient())
		{
			m_ZenLoginHasFinishedClient = false;
			m_ZenLoginForceCancel = false;
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ZenLoginBegin);
			return;
		}

		m_ZenLoginHasFinishedServer = false;
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		m_ZenLoginForceCancel = true;
	};

	protected void ZenLoginBegin()
	{
		if (!IsControlledPlayer() || GetType().Contains("_Ghost")) // Ghost = Syberia mod compatibility
			return;

		if (ZenImmersiveLoginCancelFX())
			return;

		// This is our player object and we're fine, close eyes & lie down
		ZenImmersiveLogin();
	}

	protected bool ZenImmersiveLoginCancelFX()
	{
		return m_ZenLoginForceCancel || IsRestrained() || IsUnconscious() || IsFalling() || !IsAlive() || IsSwimming();
	}

	protected void ZenImmersiveLogin()
	{
		if (!GetEmoteManager() || GetEmoteManager().IsEmotePlaying())
			return;

		// Lie down or sit if lie-down emote is unavailable (eg. player is in shallow water or a tight space)
		if (GetEmoteManager().CanPlayEmote(EmoteConstants.ID_EMOTE_LYINGDOWN))
		{
			GetEmoteManager().CreateEmoteCBFromMenu(EmoteConstants.ID_EMOTE_LYINGDOWN);
		}
		else
		{
			GetEmoteManager().CreateEmoteCBFromMenu(EmoteConstants.ID_EMOTE_SITA);
			GetEmoteManager().GetEmoteLauncher().SetForced(EmoteLauncher.FORCE_DIFFERENT);
		}

		if (m_Hud)
			m_Hud.ShowQuickbarPlayer(false);

		// Create blackout screen effect
		m_IL_m_SpawnDarkeningCurrentTime = DEFAULT_SPAWNFX_DURATION;
		m_IL_ppeEffect = PPERequester_ZenSpawnEffects.Cast(PPERequesterBank.GetRequester(PPERequester_ZenSpawnEffects));

		if (!m_IL_ppeEffect)
			return;

		m_IL_ppeEffect.SetEffectValues(200);
		GetGame().GetSoundScene().SetSoundVolume(0, 0);

		// Schedule "wake up" 6 seconds after player loads
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ZenLoginOpenEyesClient, 6000, false);
	}

	// Make player invisible if they haven't logged in yet
	override void EOnPostFrame(IEntity other, int extra)
	{
		super.EOnPostFrame(other, extra);

		// Don't apply to client player or if this player has sync'd as logged in
		if (m_ZenLoginHasFinishedClient || IsControlledPlayer())
			return;

		// Check that this isn't a loading screen character before setting it invisible
		if (GetDayZGame().GetMissionState() == DayZGame.MISSION_STATE_MAINMENU)
			return;

		// Set invisible by clearing Visible entity flag
		ClearFlags(EntityFlags.VISIBLE, true);
	};

	// Trigger eye-open fx and enable sound
	protected void ZenLoginOpenEyesClient()
	{
		// Start opening eyes fx, and slowly fade-in volume over 5 secs
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ZenLoginUpdateSpawnDarknessLevel, 1, false);
		GetGame().GetSoundScene().SetSoundVolume(g_Game.m_volume_sound, 5);
	}

	// Simulate Spawn visual effect
	protected void ZenLoginUpdateSpawnDarknessLevel()
	{
		// Check fx PPE
		if (!m_IL_ppeEffect)
			return;

		// Calculate vignette percentage
		float percentage = Math.Lerp(0, 2, 1 - ((DEFAULT_SPAWNFX_DURATION - m_IL_m_SpawnDarkeningCurrentTime) / DEFAULT_SPAWNFX_DURATION));

		// Decrease fx timer
		m_IL_m_SpawnDarkeningCurrentTime -= 1;
		m_IL_ppeEffect.SetEffectValues(percentage);

		// If animation of opening eyes is over, stop updating.
		if (percentage <= 0 || ZenImmersiveLoginCancelFX())
		{
			// If our eyes are no longer shut, remove the Spawn requester effect altogether
			if (m_IL_ppeEffect)
			{
				m_IL_ppeEffect.SetEffectValues(0);
				m_IL_ppeEffect.Stop();
				m_IL_ppeEffect = null;
			}

			// Stop update loop
			return;
		}

		// Re-schedule update loop
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ZenLoginUpdateSpawnDarknessLevel, 1, false);
	}

	void ZenImmersiveLoginFinished()
	{
		m_ZenLoginHasFinishedClient = true;

		if (GetGame().IsDedicatedServer())
		{
			m_ZenLoginHasFinishedServer = true;
			SetSynchDirty();
		}
	}

	bool ZenLoginHasFinished()
	{
		return m_ZenLoginHasFinishedClient || m_ZenLoginHasFinishedServer;
	}
};
