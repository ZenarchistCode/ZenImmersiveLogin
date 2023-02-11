// While player is 'invisible', they're logging in, so hide the target HUD or else they'll know their character is using the lie-down emote
modded class ActionTargetsCursor
{
	override void Update()
	{
		if (m_Player && m_Player.m_ImmersiveLoginInvisible)
		{
			m_Root.Show(false);
			m_CachedObject.Invalidate();
			return;
		}

		super.Update();
	};
};