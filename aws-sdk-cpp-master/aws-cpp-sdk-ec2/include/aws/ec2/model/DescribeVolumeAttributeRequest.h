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
#include <aws/ec2/EC2_EXPORTS.h>
#include <aws/ec2/EC2Request.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/ec2/model/VolumeAttributeName.h>

namespace Aws
{
namespace EC2
{
namespace Model
{

  /**
   * <p>Contains the parameters for DescribeVolumeAttribute.</p>
   */
  class AWS_EC2_API DescribeVolumeAttributeRequest : public EC2Request
  {
  public:
    DescribeVolumeAttributeRequest();
    Aws::String SerializePayload() const override;

    /**
     * <p>Checks whether you have the required permissions for the action, without
     * actually making the request, and provides an error response. If you have the
     * required permissions, the error response is <code>DryRunOperation</code>.
     * Otherwise, it is <code>UnauthorizedOperation</code>.</p>
     */
    inline bool GetDryRun() const{ return m_dryRun; }

    /**
     * <p>Checks whether you have the required permissions for the action, without
     * actually making the request, and provides an error response. If you have the
     * required permissions, the error response is <code>DryRunOperation</code>.
     * Otherwise, it is <code>UnauthorizedOperation</code>.</p>
     */
    inline void SetDryRun(bool value) { m_dryRunHasBeenSet = true; m_dryRun = value; }

    /**
     * <p>Checks whether you have the required permissions for the action, without
     * actually making the request, and provides an error response. If you have the
     * required permissions, the error response is <code>DryRunOperation</code>.
     * Otherwise, it is <code>UnauthorizedOperation</code>.</p>
     */
    inline DescribeVolumeAttributeRequest& WithDryRun(bool value) { SetDryRun(value); return *this;}

    /**
     * <p>The ID of the volume.</p>
     */
    inline const Aws::String& GetVolumeId() const{ return m_volumeId; }

    /**
     * <p>The ID of the volume.</p>
     */
    inline void SetVolumeId(const Aws::String& value) { m_volumeIdHasBeenSet = true; m_volumeId = value; }

    /**
     * <p>The ID of the volume.</p>
     */
    inline void SetVolumeId(Aws::String&& value) { m_volumeIdHasBeenSet = true; m_volumeId = value; }

    /**
     * <p>The ID of the volume.</p>
     */
    inline void SetVolumeId(const char* value) { m_volumeIdHasBeenSet = true; m_volumeId.assign(value); }

    /**
     * <p>The ID of the volume.</p>
     */
    inline DescribeVolumeAttributeRequest& WithVolumeId(const Aws::String& value) { SetVolumeId(value); return *this;}

    /**
     * <p>The ID of the volume.</p>
     */
    inline DescribeVolumeAttributeRequest& WithVolumeId(Aws::String&& value) { SetVolumeId(value); return *this;}

    /**
     * <p>The ID of the volume.</p>
     */
    inline DescribeVolumeAttributeRequest& WithVolumeId(const char* value) { SetVolumeId(value); return *this;}

    /**
     * <p>The instance attribute.</p>
     */
    inline const VolumeAttributeName& GetAttribute() const{ return m_attribute; }

    /**
     * <p>The instance attribute.</p>
     */
    inline void SetAttribute(const VolumeAttributeName& value) { m_attributeHasBeenSet = true; m_attribute = value; }

    /**
     * <p>The instance attribute.</p>
     */
    inline void SetAttribute(VolumeAttributeName&& value) { m_attributeHasBeenSet = true; m_attribute = value; }

    /**
     * <p>The instance attribute.</p>
     */
    inline DescribeVolumeAttributeRequest& WithAttribute(const VolumeAttributeName& value) { SetAttribute(value); return *this;}

    /**
     * <p>The instance attribute.</p>
     */
    inline DescribeVolumeAttributeRequest& WithAttribute(VolumeAttributeName&& value) { SetAttribute(value); return *this;}

  private:
    bool m_dryRun;
    bool m_dryRunHasBeenSet;
    Aws::String m_volumeId;
    bool m_volumeIdHasBeenSet;
    VolumeAttributeName m_attribute;
    bool m_attributeHasBeenSet;
  };

} // namespace Model
} // namespace EC2
} // namespace Aws