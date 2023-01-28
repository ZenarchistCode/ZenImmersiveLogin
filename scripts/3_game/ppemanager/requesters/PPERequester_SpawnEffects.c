class PPERequester_ZenSpawnEffects extends PPERequester_GameplayBase
{
	protected ref array<float> m_OverlayColor;

	void SetEffectValues(float percentage)
	{
		if (!IsRequesterRunning())
			Start();

		m_OverlayColor = { 0.0, 0.0, 0.0, 1.0 };
		if (percentage > 0.0)
			m_OverlayColor = { 0.1, 0.1, 0.1, 1.0 };

		// Set blur intensity
		SetTargetValueFloat(PostProcessEffectType.GaussFilter, PPEGaussFilter.PARAM_INTENSITY, true, percentage * 0.2, PPEGaussFilter.L_0_SHOCK, PPOperators.ADD_RELATIVE);
		// Set vignette intensity
		SetTargetValueFloat(PostProcessEffectType.Glow, PPEGlow.PARAM_VIGNETTE, false, percentage, PPEGlow.L_25_SHOCK, PPOperators.ADD);
		// Set black screen overlay color and intensity
		SetTargetValueFloat(PostProcessEffectType.Glow, PPEGlow.PARAM_OVERLAYFACTOR, true, percentage * 0.16, PPEGlow.L_20_SHOCK, PPOperators.HIGHEST);
		SetTargetValueColor(PostProcessEffectType.Glow, PPEGlow.PARAM_OVERLAYCOLOR, m_OverlayColor, PPEGlow.L_21_SHOCK, PPOperators.SET);
	}
}