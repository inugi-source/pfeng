/* =========================================================================
 *  Copyright 2021 NXP
 *
 *  SPDX-License-Identifier: GPL-2.0
 *
 * ========================================================================= */

#include "pfe_cfg.h"
#include "pfe_ct.h"
#include "oal.h"
#include "pfe_class.h"
#include "pfe_hw_feature.h"
#include "pfe_feature_mgr.h"
#include "pfe_platform.h"
#include "pfe_cbus.h"

struct pfe_hw_feature_tag
{
	/* Similar to pfe_ct_feature_desc_t */
	char *name;	/* Feature name */
	char *description;	/* Feature description */
	pfe_ct_feature_flags_t flags;
	uint8_t def_val;	/* Enable/disable default value used for runtime configuration */
	uint8_t	val;
};

/**
 * @brief Creates a feature instance
 * @return The created feature instance or NULL in case of failure
 */
static pfe_hw_feature_t *pfe_hw_feature_create(char *name, char *descr, pfe_ct_feature_flags_t flags, uint8_t def_val)
{
	pfe_hw_feature_t *feature;
	feature = oal_mm_malloc(sizeof(pfe_hw_feature_t));
	if(NULL != feature)
	{
		(void)memset(feature, 0U, sizeof(pfe_hw_feature_t));
		feature->name = name;
		feature->description = descr;
		feature->flags = flags;
		feature->def_val = def_val;
	}
	else
	{
		NXP_LOG_ERROR("Cannot allocate %u bytes of memory for feature\n", (uint_t)sizeof(pfe_hw_feature_t));
	}
	return feature;
}

/**
 * @brief Destroys a feature instance previously created by pfe_hw_feature_create()
 * @param[in] feature Previously created feature
 */
void pfe_hw_feature_destroy(const pfe_hw_feature_t *feature)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(NULL == feature))
	{
		NXP_LOG_ERROR("NULL argument received\n");
		return;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */
	oal_mm_free(feature);
}

errno_t pfe_hw_feature_init_all(uint32_t *cbus_base, pfe_hw_feature_t ***hw_features, uint32_t *hw_features_count)
{
	uint32_t val;
	pfe_hw_feature_t *feature;
	pfe_hw_feature_t **hw_features_arr;
	bool_t on_g3 = FALSE;

	hw_features_arr = oal_mm_malloc(1U * sizeof(pfe_hw_feature_t *));

	feature = pfe_hw_feature_create(PFE_HW_FEATURE_RUN_ON_G3, "Active if running on S32G3", F_PRESENT, 0);
	if (NULL != feature)
	{
		/*      Detect S32G silicon version */
		val = hal_read32((void *)(CBUS_GLOBAL_CSR_BASE_ADDR + WSP_VERSION + (addr_t)cbus_base));
		if(0x00050300 == val)
		{       /* S32G2 */
			NXP_LOG_INFO("Silicon S32G2\n");
		}
		else if(0x00000101 == val)
		{       /* S32G3 */
			on_g3 = TRUE;
			NXP_LOG_INFO("Silicon S32G3\n");
		}
		else
		{       /* Unknown */
			NXP_LOG_ERROR("Silicon HW version is unknown: 0x%x\n", val);
		}

		pfe_hw_feature_set_val(feature, on_g3);
		hw_features_arr[0] = feature;

	}
	else
	{
		return ENOMEM;
	}

	*hw_features = hw_features_arr;
	*hw_features_count = 1U;

	return EOK;
}

/**
 * @brief Returns name of the feature
 * @param[in] feature Feature to be read.
 * @param[out] name The feature name to be read.
 * @return EOK or an error code.
 */
errno_t pfe_hw_feature_get_name(const pfe_hw_feature_t *feature, const char **name)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((NULL == feature)||(NULL == name)))
	{
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */
	*name = feature->name;
	return EOK;
}

/**
 * @brief Returns the feature description provide by the firmware.
 * @param[in] feature Feature to be read.
 * @param[out] desc Descripton of the feature
 * @return EOK or an error code.
 */
errno_t pfe_hw_feature_get_desc(const pfe_hw_feature_t *feature, const char **desc)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((NULL == feature)||(NULL ==desc)))
	{
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */
	*desc = feature->description;
	return EOK;
}

/**
 * @brief Reads the flags of the feature
 * @param[in] feature Feature to be read
 * @param[out] flags Value of the feature flags
 * @return EOK or an error code.
 */
errno_t pfe_hw_feature_get_flags(const pfe_hw_feature_t *feature, pfe_ct_feature_flags_t *flags)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((NULL == feature)||(NULL==flags)))
	{
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */
	*flags = feature->flags;
	return EOK;
}

/**
 * @brief Reads the default value of the feature i.e. initial value set by the FW
 * @param[in] feature Feature to read the value
 * @param[out] def_val The read default value.
 * @return EOK or an error code.
 */
errno_t pfe_hw_feature_get_def_val(const pfe_hw_feature_t *feature, uint8_t *def_val)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((NULL == feature)||(NULL == def_val)))
	{
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */
	*def_val = feature->def_val;
	return EOK;
}

/**
 * @brief Reads value of the feature enable variable
 * @param[in] Feature to read the value
 * @param[out] val Value read from the DMEM
 * @return EOK or an error code.
 */
errno_t pfe_hw_feature_get_val(const pfe_hw_feature_t *feature, uint8_t *val)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((NULL == feature)||(NULL == val)))
	{
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	*val = feature->val;
	return EOK;
}

/**
 * @brief Checks whether the given feature is in enabled state
 * @param[in] feature Feature to check the enabled state
 * @retval TRUE Feature is enabled (the enable variable value is not 0)
 * @retval FALSE Feature is disabled (or its state could not be read)
 */
bool_t pfe_hw_feature_enabled(const pfe_hw_feature_t *feature)
{
	uint8_t val;
	errno_t ret;
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(NULL == feature))
	{
		NXP_LOG_ERROR("NULL argument received\n");
		return FALSE;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	ret = pfe_hw_feature_get_val(feature, &val);
	if(EOK != ret)
	{
		return FALSE;
	}
	if(0U != val)
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * @brief Sets value of the feature enable variable in the DMEM
 * @param[in] feature Feature to set the value
 * @param[in] val Value to be set
 * @return EOK or an error code.
 */
errno_t pfe_hw_feature_set_val(pfe_hw_feature_t *feature, uint8_t val)
{
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(NULL == feature))
	{
		NXP_LOG_ERROR("NULL argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */
	feature->val = val;
	return EOK;
}
