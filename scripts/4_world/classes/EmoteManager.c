modded class EmoteManager
{
	// Don't allow login emote to be interrupted until after login effect has triggered
	override bool InterruptGestureCheck()
	{
		if (m_Player && !m_Player.ZenLoginHasFinished())
		{
			return false;
		}

		return super.InterruptGestureCheck();
	};

	// Called on both client and server. Server will sync login status to other clients
	override void Update(float deltaT)
	{
		super.Update(deltaT);

		if (m_Player.ZenLoginHasFinished())
			return;

		if (m_Callback && m_CurrentGestureID == EmoteConstants.ID_EMOTE_LYINGDOWN)
		{
			if (m_Callback.GetState() == 2) // Lying down on ground part of animation
			{
				m_Player.ZenImmersiveLoginFinished();
			}
		}
		else
		if (m_Player.GetSimulationTimeStamp() >= 150) // 110 is how long it should take to lie down server-side
		{
			// Failsafe, no emote has been performed within reasonable period of time - set player as 'logged in'
			m_Player.ZenImmersiveLoginFinished();
		}
	};
};
