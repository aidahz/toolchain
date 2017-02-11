﻿/*
* Copyright 2010-2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <aws/route53domains/Route53Domains_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>

namespace Aws
{
namespace Route53Domains
{
namespace Model
{
  enum class ContactType
  {
    NOT_SET,
    PERSON,
    COMPANY,
    ASSOCIATION,
    PUBLIC_BODY,
    RESELLER
  };

namespace ContactTypeMapper
{
AWS_ROUTE53DOMAINS_API ContactType GetContactTypeForName(const Aws::String& name);

AWS_ROUTE53DOMAINS_API Aws::String GetNameForContactType(ContactType value);
} // namespace ContactTypeMapper
} // namespace Model
} // namespace Route53Domains
} // namespace Aws