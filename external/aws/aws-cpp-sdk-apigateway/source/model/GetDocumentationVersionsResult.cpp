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

#include <aws/apigateway/model/GetDocumentationVersionsResult.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/AmazonWebServiceResult.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/utils/UnreferencedParam.h>

#include <utility>

using namespace Aws::APIGateway::Model;
using namespace Aws::Utils::Json;
using namespace Aws::Utils;
using namespace Aws;

GetDocumentationVersionsResult::GetDocumentationVersionsResult()
{
}

GetDocumentationVersionsResult::GetDocumentationVersionsResult(const Aws::AmazonWebServiceResult<JsonValue>& result)
{
  *this = result;
}

GetDocumentationVersionsResult& GetDocumentationVersionsResult::operator =(const Aws::AmazonWebServiceResult<JsonValue>& result)
{
  JsonView jsonValue = result.GetPayload().View();
  if(jsonValue.ValueExists("position"))
  {
    m_position = jsonValue.GetString("position");

  }

  if(jsonValue.ValueExists("item"))
  {
    Array<JsonView> itemJsonList = jsonValue.GetArray("item");
    for(unsigned itemIndex = 0; itemIndex < itemJsonList.GetLength(); ++itemIndex)
    {
      m_items.push_back(itemJsonList[itemIndex].AsObject());
    }
  }



  return *this;
}