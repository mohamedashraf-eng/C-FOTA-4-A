/**
 ******************************************************************************
 * @file           : wXLLSec_SHA256.h
 * @brief          : wXLLSec_SHA256 public declaration file
 ******************************************************************************
 * @attention
 *
 * All rights reserved.
 *
 * Copyright (c) 2023 Mohamed Ashraf Wx
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
/** @brief Header guard */
#if !defined(__WXLLSEC_SHA256_H__)
#define __WXLLSEC_SHA256_H__

/** @brief C++ name mangle guard */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus */

#include "../include/Std_Types.h"

/**
 * ===============================================================================================
 *   > Public Macros
 * ===============================================================================================
 */

#define WXLLSEC_SHA256_INIT_VALUE (NULL)

/**
 * ===============================================================================================
 *   > Public Enums
 * ===============================================================================================
 */

/**
 * @brief Enum representing the possible error states for SHA-256 operations.
 */
enum __sha256_ErrorState {
	SHA256_SUCCESS = 0,    /**< Operation completed successfully. */
	SHA256_NULL_POINTER,   /**< Null pointer encountered. */
	SHA256_INVALID_LENGTH, /**< Invalid length specified. */
	SHA256_UNKNOWN_ERROR   /**< Unknown error occurred. */
};

/**
 * ===============================================================================================
 *   > Public Datatypes
 * ===============================================================================================
 */

/**
 * @brief Structure representing the SHA-256 context.
 */
typedef struct __sha256_ctx sha256_ctx_t;

/**
 * @brief Typedef for the SHA-256 error state enumeration.
 */
typedef enum __sha256_ErrorState sha256_ErrorState_t;

/**
 * @brief Typedef for the SHA-256 digest.
 */
typedef UINT8 sha256_digest_t[32u];

/**
 * ===============================================================================================
 *   > Public Function Declaration
 * ===============================================================================================
 */

/**
 * @brief Initializes the SHA-256 context.
 *
 * @param ctx Pointer to the SHA-256 context.
 * @return SHA-256 error state.
 */
sha256_ErrorState_t wXLLSecSHA256_Init(sha256_ctx_t **ctx);

/**
 * @brief Updates the SHA-256 context with data.
 *
 * @param ctx Pointer to the SHA-256 context.
 * @param data Pointer to the input data.
 * @param length Length of the input data.
 * @return SHA-256 error state.
 */
sha256_ErrorState_t wXLLSecSHA256_Update(sha256_ctx_t **ctx, __CONST UINT8PTR data, __CONST UINT64 length);

/**
 * @brief Finalizes the SHA-256 hash and obtains the result.
 *
 * @param ctx Pointer to the SHA-256 context.
 * @param hash Pointer to the SHA-256 digest.
 * @return SHA-256 error state.
 */
sha256_ErrorState_t wXLLSecSHA256_Final(sha256_ctx_t **ctx, sha256_digest_t hash);

/**
 * @brief 
 * 
 * @param diest 
 * @return sha256_ErrorState_t 
 */
sha256_ErrorState_t wXLLSecSHA256_PrintDigest(__CONST sha256_digest_t digest);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus */
#endif /* __WXLLSEC_SHA256_H__ */
