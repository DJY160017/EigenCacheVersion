﻿/*
* Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License").
* You may not use this file except in compliance with the License.
* A copy of the License is located at
*
*  http://aws.amazon.com/apache2.0
*
* or in the "license" file accompanying this file. This file is distributed
* on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
* express or implied. See the License for the specific language governing
* permissions and limitations under the License.
*/

#pragma once
#include <aws/servicecatalog/ServiceCatalog_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <utility>

namespace Aws
{
namespace Utils
{
namespace Json
{
  class JsonValue;
  class JsonView;
} // namespace Json
} // namespace Utils
namespace ServiceCatalog
{
namespace Model
{

  /**
   * <p>The search filter to use when listing history records.</p><p><h3>See
   * Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/servicecatalog-2015-12-10/ListRecordHistorySearchFilter">AWS
   * API Reference</a></p>
   */
  class AWS_SERVICECATALOG_API ListRecordHistorySearchFilter
  {
  public:
    ListRecordHistorySearchFilter();
    ListRecordHistorySearchFilter(Aws::Utils::Json::JsonView jsonValue);
    ListRecordHistorySearchFilter& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     * <p>The filter key.</p> <ul> <li> <p> <code>product</code> - Filter results based
     * on the specified product identifier.</p> </li> <li> <p>
     * <code>provisionedproduct</code> - Filter results based on the provisioned
     * product identifier.</p> </li> </ul>
     */
    inline const Aws::String& GetKey() const{ return m_key; }

    /**
     * <p>The filter key.</p> <ul> <li> <p> <code>product</code> - Filter results based
     * on the specified product identifier.</p> </li> <li> <p>
     * <code>provisionedproduct</code> - Filter results based on the provisioned
     * product identifier.</p> </li> </ul>
     */
    inline void SetKey(const Aws::String& value) { m_keyHasBeenSet = true; m_key = value; }

    /**
     * <p>The filter key.</p> <ul> <li> <p> <code>product</code> - Filter results based
     * on the specified product identifier.</p> </li> <li> <p>
     * <code>provisionedproduct</code> - Filter results based on the provisioned
     * product identifier.</p> </li> </ul>
     */
    inline void SetKey(Aws::String&& value) { m_keyHasBeenSet = true; m_key = std::move(value); }

    /**
     * <p>The filter key.</p> <ul> <li> <p> <code>product</code> - Filter results based
     * on the specified product identifier.</p> </li> <li> <p>
     * <code>provisionedproduct</code> - Filter results based on the provisioned
     * product identifier.</p> </li> </ul>
     */
    inline void SetKey(const char* value) { m_keyHasBeenSet = true; m_key.assign(value); }

    /**
     * <p>The filter key.</p> <ul> <li> <p> <code>product</code> - Filter results based
     * on the specified product identifier.</p> </li> <li> <p>
     * <code>provisionedproduct</code> - Filter results based on the provisioned
     * product identifier.</p> </li> </ul>
     */
    inline ListRecordHistorySearchFilter& WithKey(const Aws::String& value) { SetKey(value); return *this;}

    /**
     * <p>The filter key.</p> <ul> <li> <p> <code>product</code> - Filter results based
     * on the specified product identifier.</p> </li> <li> <p>
     * <code>provisionedproduct</code> - Filter results based on the provisioned
     * product identifier.</p> </li> </ul>
     */
    inline ListRecordHistorySearchFilter& WithKey(Aws::String&& value) { SetKey(std::move(value)); return *this;}

    /**
     * <p>The filter key.</p> <ul> <li> <p> <code>product</code> - Filter results based
     * on the specified product identifier.</p> </li> <li> <p>
     * <code>provisionedproduct</code> - Filter results based on the provisioned
     * product identifier.</p> </li> </ul>
     */
    inline ListRecordHistorySearchFilter& WithKey(const char* value) { SetKey(value); return *this;}


    /**
     * <p>The filter value.</p>
     */
    inline const Aws::String& GetValue() const{ return m_value; }

    /**
     * <p>The filter value.</p>
     */
    inline void SetValue(const Aws::String& value) { m_valueHasBeenSet = true; m_value = value; }

    /**
     * <p>The filter value.</p>
     */
    inline void SetValue(Aws::String&& value) { m_valueHasBeenSet = true; m_value = std::move(value); }

    /**
     * <p>The filter value.</p>
     */
    inline void SetValue(const char* value) { m_valueHasBeenSet = true; m_value.assign(value); }

    /**
     * <p>The filter value.</p>
     */
    inline ListRecordHistorySearchFilter& WithValue(const Aws::String& value) { SetValue(value); return *this;}

    /**
     * <p>The filter value.</p>
     */
    inline ListRecordHistorySearchFilter& WithValue(Aws::String&& value) { SetValue(std::move(value)); return *this;}

    /**
     * <p>The filter value.</p>
     */
    inline ListRecordHistorySearchFilter& WithValue(const char* value) { SetValue(value); return *this;}

  private:

    Aws::String m_key;
    bool m_keyHasBeenSet;

    Aws::String m_value;
    bool m_valueHasBeenSet;
  };

} // namespace Model
} // namespace ServiceCatalog
} // namespace Aws
