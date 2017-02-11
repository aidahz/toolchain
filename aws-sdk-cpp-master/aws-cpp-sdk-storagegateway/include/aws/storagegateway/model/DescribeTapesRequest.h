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
#include <aws/storagegateway/StorageGateway_EXPORTS.h>
#include <aws/storagegateway/StorageGatewayRequest.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSVector.h>

namespace Aws
{
namespace StorageGateway
{
namespace Model
{

  /**
   * <p>DescribeTapesInput</p>
   */
  class AWS_STORAGEGATEWAY_API DescribeTapesRequest : public StorageGatewayRequest
  {
  public:
    DescribeTapesRequest();
    Aws::String SerializePayload() const override;

    Aws::Http::HeaderValueCollection GetRequestSpecificHeaders() const override;

    
    inline const Aws::String& GetGatewayARN() const{ return m_gatewayARN; }

    
    inline void SetGatewayARN(const Aws::String& value) { m_gatewayARNHasBeenSet = true; m_gatewayARN = value; }

    
    inline void SetGatewayARN(Aws::String&& value) { m_gatewayARNHasBeenSet = true; m_gatewayARN = value; }

    
    inline void SetGatewayARN(const char* value) { m_gatewayARNHasBeenSet = true; m_gatewayARN.assign(value); }

    
    inline DescribeTapesRequest& WithGatewayARN(const Aws::String& value) { SetGatewayARN(value); return *this;}

    
    inline DescribeTapesRequest& WithGatewayARN(Aws::String&& value) { SetGatewayARN(value); return *this;}

    
    inline DescribeTapesRequest& WithGatewayARN(const char* value) { SetGatewayARN(value); return *this;}

    /**
     * <p>Specifies one or more unique Amazon Resource Names (ARNs) that represent the
     * virtual tapes you want to describe. If this parameter is not specified, AWS
     * Storage Gateway returns a description of all virtual tapes associated with the
     * specified gateway.</p>
     */
    inline const Aws::Vector<Aws::String>& GetTapeARNs() const{ return m_tapeARNs; }

    /**
     * <p>Specifies one or more unique Amazon Resource Names (ARNs) that represent the
     * virtual tapes you want to describe. If this parameter is not specified, AWS
     * Storage Gateway returns a description of all virtual tapes associated with the
     * specified gateway.</p>
     */
    inline void SetTapeARNs(const Aws::Vector<Aws::String>& value) { m_tapeARNsHasBeenSet = true; m_tapeARNs = value; }

    /**
     * <p>Specifies one or more unique Amazon Resource Names (ARNs) that represent the
     * virtual tapes you want to describe. If this parameter is not specified, AWS
     * Storage Gateway returns a description of all virtual tapes associated with the
     * specified gateway.</p>
     */
    inline void SetTapeARNs(Aws::Vector<Aws::String>&& value) { m_tapeARNsHasBeenSet = true; m_tapeARNs = value; }

    /**
     * <p>Specifies one or more unique Amazon Resource Names (ARNs) that represent the
     * virtual tapes you want to describe. If this parameter is not specified, AWS
     * Storage Gateway returns a description of all virtual tapes associated with the
     * specified gateway.</p>
     */
    inline DescribeTapesRequest& WithTapeARNs(const Aws::Vector<Aws::String>& value) { SetTapeARNs(value); return *this;}

    /**
     * <p>Specifies one or more unique Amazon Resource Names (ARNs) that represent the
     * virtual tapes you want to describe. If this parameter is not specified, AWS
     * Storage Gateway returns a description of all virtual tapes associated with the
     * specified gateway.</p>
     */
    inline DescribeTapesRequest& WithTapeARNs(Aws::Vector<Aws::String>&& value) { SetTapeARNs(value); return *this;}

    /**
     * <p>Specifies one or more unique Amazon Resource Names (ARNs) that represent the
     * virtual tapes you want to describe. If this parameter is not specified, AWS
     * Storage Gateway returns a description of all virtual tapes associated with the
     * specified gateway.</p>
     */
    inline DescribeTapesRequest& AddTapeARNs(const Aws::String& value) { m_tapeARNsHasBeenSet = true; m_tapeARNs.push_back(value); return *this; }

    /**
     * <p>Specifies one or more unique Amazon Resource Names (ARNs) that represent the
     * virtual tapes you want to describe. If this parameter is not specified, AWS
     * Storage Gateway returns a description of all virtual tapes associated with the
     * specified gateway.</p>
     */
    inline DescribeTapesRequest& AddTapeARNs(Aws::String&& value) { m_tapeARNsHasBeenSet = true; m_tapeARNs.push_back(value); return *this; }

    /**
     * <p>Specifies one or more unique Amazon Resource Names (ARNs) that represent the
     * virtual tapes you want to describe. If this parameter is not specified, AWS
     * Storage Gateway returns a description of all virtual tapes associated with the
     * specified gateway.</p>
     */
    inline DescribeTapesRequest& AddTapeARNs(const char* value) { m_tapeARNsHasBeenSet = true; m_tapeARNs.push_back(value); return *this; }

    /**
     * <p>A marker value, obtained in a previous call to <code>DescribeTapes</code>.
     * This marker indicates which page of results to retrieve. </p> <p>If not
     * specified, the first page of results is retrieved.</p>
     */
    inline const Aws::String& GetMarker() const{ return m_marker; }

    /**
     * <p>A marker value, obtained in a previous call to <code>DescribeTapes</code>.
     * This marker indicates which page of results to retrieve. </p> <p>If not
     * specified, the first page of results is retrieved.</p>
     */
    inline void SetMarker(const Aws::String& value) { m_markerHasBeenSet = true; m_marker = value; }

    /**
     * <p>A marker value, obtained in a previous call to <code>DescribeTapes</code>.
     * This marker indicates which page of results to retrieve. </p> <p>If not
     * specified, the first page of results is retrieved.</p>
     */
    inline void SetMarker(Aws::String&& value) { m_markerHasBeenSet = true; m_marker = value; }

    /**
     * <p>A marker value, obtained in a previous call to <code>DescribeTapes</code>.
     * This marker indicates which page of results to retrieve. </p> <p>If not
     * specified, the first page of results is retrieved.</p>
     */
    inline void SetMarker(const char* value) { m_markerHasBeenSet = true; m_marker.assign(value); }

    /**
     * <p>A marker value, obtained in a previous call to <code>DescribeTapes</code>.
     * This marker indicates which page of results to retrieve. </p> <p>If not
     * specified, the first page of results is retrieved.</p>
     */
    inline DescribeTapesRequest& WithMarker(const Aws::String& value) { SetMarker(value); return *this;}

    /**
     * <p>A marker value, obtained in a previous call to <code>DescribeTapes</code>.
     * This marker indicates which page of results to retrieve. </p> <p>If not
     * specified, the first page of results is retrieved.</p>
     */
    inline DescribeTapesRequest& WithMarker(Aws::String&& value) { SetMarker(value); return *this;}

    /**
     * <p>A marker value, obtained in a previous call to <code>DescribeTapes</code>.
     * This marker indicates which page of results to retrieve. </p> <p>If not
     * specified, the first page of results is retrieved.</p>
     */
    inline DescribeTapesRequest& WithMarker(const char* value) { SetMarker(value); return *this;}

    /**
     * <p>Specifies that the number of virtual tapes described be limited to the
     * specified number.</p> <note><p>Amazon Web Services may impose its own limit, if
     * this field is not set.</p> </note>
     */
    inline int GetLimit() const{ return m_limit; }

    /**
     * <p>Specifies that the number of virtual tapes described be limited to the
     * specified number.</p> <note><p>Amazon Web Services may impose its own limit, if
     * this field is not set.</p> </note>
     */
    inline void SetLimit(int value) { m_limitHasBeenSet = true; m_limit = value; }

    /**
     * <p>Specifies that the number of virtual tapes described be limited to the
     * specified number.</p> <note><p>Amazon Web Services may impose its own limit, if
     * this field is not set.</p> </note>
     */
    inline DescribeTapesRequest& WithLimit(int value) { SetLimit(value); return *this;}

  private:
    Aws::String m_gatewayARN;
    bool m_gatewayARNHasBeenSet;
    Aws::Vector<Aws::String> m_tapeARNs;
    bool m_tapeARNsHasBeenSet;
    Aws::String m_marker;
    bool m_markerHasBeenSet;
    int m_limit;
    bool m_limitHasBeenSet;
  };

} // namespace Model
} // namespace StorageGateway
} // namespace Aws