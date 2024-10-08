//
//  ElementXMLTest.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/26/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "ElementXMLTest.hpp"

#include <string>

#include "ElementXML.h"

const std::string tag1("tag1");
const std::string att11("att11");
const std::string val11("val11");
const std::string att12("att12");
const std::string val12("val12");
const std::string data1("This is a string of data");
const std::string comment1("This is a comment that contains <>");

const std::string tag2("tag2");
const std::string att21("type");
const std::string val21("call me < & now");
const std::string data2("another string <s> of \"data\"");

const std::string tag3("tag3");

const std::string tag4("tag4");
const std::string att41("att41");
const std::string val41("longer value");
const std::string data4("some data");

const std::string tag5("tag5");

const int ElementXMLTest::BUFFER_LENGTH = 10;

void ElementXMLTest::setUp() {
  agent = nullptr;

  for (int i = 0; i < BUFFER_LENGTH; i++) {
    buffer[i] = static_cast<char>(i * 30);
  }
}

bool ElementXMLTest::verifyBuffer(const char* newBuffer) const {
  for (int i = 0; i < BUFFER_LENGTH; i++) {
    if (buffer[i] != newBuffer[i]) {
      return false;
    }
  }
  return true;
}

void ElementXMLTest::tearDown(bool caught) {
  for (int i = 0; i < BUFFER_LENGTH; i++) {
    buffer[i] = static_cast<char>(0);
  }
}

soarxml::ElementXML* ElementXMLTest::createXML1() {
  soarxml::ElementXML* pXML1 = new soarxml::ElementXML();
  pXML1->SetTagName(tag1.c_str());
  pXML1->AddAttribute(att11.c_str(), val11.c_str());
  pXML1->AddAttribute(att12.c_str(), val12.c_str());
  pXML1->SetCharacterData(data1.c_str());
  pXML1->SetComment(comment1.c_str());
  return pXML1;
}

soarxml::ElementXML* ElementXMLTest::createXML2() {
  soarxml::ElementXML* pXML2 = new soarxml::ElementXML();
  pXML2->SetTagName(tag2.c_str());
  pXML2->AddAttribute(att21.c_str(), val21.c_str());
  pXML2->SetCharacterData(data2.c_str());
  return pXML2;
}

soarxml::ElementXML* ElementXMLTest::createXML3() {
  soarxml::ElementXML* pXML3 = new soarxml::ElementXML();
  pXML3->SetTagName(tag3.c_str());
  return pXML3;
}

soarxml::ElementXML* ElementXMLTest::createXML4() {
  soarxml::ElementXML* pXML4 = new soarxml::ElementXML();
  pXML4->SetTagName(tag4.c_str());
  pXML4->AddAttribute(att41.c_str(), val41.c_str());
  pXML4->SetCharacterData(data4.c_str());
  return pXML4;
}

soarxml::ElementXML* ElementXMLTest::createXML5() {
  soarxml::ElementXML* pXML5 = new soarxml::ElementXML();
  pXML5->SetTagName(tag5.c_str());
  pXML5->SetBinaryCharacterData(buffer, BUFFER_LENGTH);
  return pXML5;
}

void ElementXMLTest::testSimple() {
  soarxml::ElementXML* pXML1 = createXML1();

  assertTrue(pXML1->GetNumberAttributes() == 2);
  assertTrue(pXML1->GetAttribute(att11.c_str()) != NULL);
  assertTrue(std::string(pXML1->GetAttribute(att11.c_str())) == val11);
  assertTrue(pXML1->GetAttribute(att12.c_str()) != NULL);
  assertTrue(std::string(pXML1->GetAttribute(att12.c_str())) == val12);
  assertTrue(pXML1->GetAttribute("not att") == NULL);
  assertTrue(pXML1->GetTagName() != NULL);
  assertTrue(std::string(pXML1->GetTagName()) == tag1);
  assertTrue(pXML1->GetCharacterData() != NULL);
  assertTrue(std::string(pXML1->GetCharacterData()) == data1);
  assertTrue(pXML1->GetUseCData() == false);
  assertTrue(pXML1->GetComment() != NULL);
  assertTrue(std::string(pXML1->GetComment()) == comment1);

  delete pXML1;
}

void ElementXMLTest::testChildren() {
  soarxml::ElementXML* pXML1 = createXML1();
  soarxml::ElementXML* pXML2 = createXML2();
  soarxml::ElementXML* pXML3 = createXML3();
  soarxml::ElementXML* pXML4 = createXML4();

  pXML4->AddChild(pXML1);
  pXML4->AddChild(pXML2);
  pXML4->AddChild(pXML3);

  assertTrue(pXML4->GetNumberChildren() == 3);

  soarxml::ElementXML child0(NULL);
  soarxml::ElementXML const* pChild0 = &child0;

  assertTrue(pXML4->GetChild(&child0, 0));
  assertTrue(pChild0->GetTagName() != NULL);
  assertTrue(std::string(pChild0->GetTagName()) == tag1);
  assertTrue(pChild0->GetCharacterData() != NULL);
  assertTrue(std::string(pChild0->GetCharacterData()) == data1);

  assertTrue(pChild0->GetNumberAttributes() == 2);
  assertTrue(pChild0->GetNumberChildren() == 0);

  // Let's put this one on the heap so we can control when we delete it.
  soarxml::ElementXML* pChild1Object = new soarxml::ElementXML(NULL);
  soarxml::ElementXML const* pChild1 = pChild1Object;

  assertTrue(pXML4->GetChild(pChild1Object, 1));
  assertTrue(pChild1->GetTagName() != NULL);
  assertTrue(std::string(pChild1->GetTagName()) == tag2);
  assertTrue(pChild1->GetCharacterData() != NULL);
  assertTrue(std::string(pChild1->GetCharacterData()) == data2);
  assertTrue(pChild0->GetNumberChildren() == 0);
  assertTrue(pChild1->GetAttribute(att21.c_str()) != NULL);
  assertTrue(std::string(pChild1->GetAttribute(att21.c_str())) == val21);

  //// This test is because I read online about looking up an element in an
  /// empty / map causing an exception.  Need to make sure that doesn't happen
  /// in our / attribute map implementation.
  soarxml::ElementXML child2(NULL);
  soarxml::ElementXML const* pChild2 = &child2;
  assertTrue(pXML4->GetChild(&child2, 2));
  assertTrue(pChild2->GetTagName() != NULL);
  assertTrue(std::string(pChild2->GetTagName()) == tag3);
  assertTrue(pChild2->GetAttribute("missing") == NULL);

  soarxml::ElementXML test;
  assertTrue(pXML4->GetChild(&test, 3) == 0);
  assertTrue(pXML4->GetChild(&test, -3) == 0);

  // Create an XML string and print it out
  char* pStr = pXML4->GenerateXMLString(true);
  // printf(pStr) ;
  // printf("\n") ;
  pXML4->DeleteString(pStr);

  // Let's play a game.
  // Create another object pointing at the same internal handle
  soarxml::ElementXML* pChild1Alt =
      new soarxml::ElementXML(pChild1->GetXMLHandle());
  pChild1Alt->AddRefOnHandle();

  // Delete the entire tree, releasing refs on the children
  delete pXML4;

  // We have to delete this other reference into the tree or its
  // not a proper test of pChild1Alt.  (If pChild1Object was on the stack
  // it wouldn't be deleted yet and of course we could talk to the child).
  delete pChild1Object;

  // Since we added a ref to pChild1 it should still exist
  assertTrue(pChild1Alt->GetTagName() != NULL);
  assertTrue(std::string(pChild1Alt->GetTagName()) == tag2);
  assertTrue(pChild1Alt->ReleaseRefOnHandle() == 0);

  delete pChild1Alt;
}

void ElementXMLTest::testParse() {
  soarxml::ElementXML* pXML1 = createXML1();

  char* pStr = pXML1->GenerateXMLString(true);

  soarxml::ElementXML* pXML2 = soarxml::ElementXML::ParseXMLFromString(pStr);

  assertTrue(pXML2 != NULL);
  assertTrue(pXML2->GetNumberAttributes() == 2);
  assertTrue(pXML2->GetAttribute(att11.c_str()) != NULL);
  assertTrue(std::string(pXML2->GetAttribute(att11.c_str())) == val11);
  assertTrue(pXML2->GetAttribute(att12.c_str()) != NULL);
  assertTrue(std::string(pXML2->GetAttribute(att12.c_str())) == val12);
  assertTrue(pXML2->GetAttribute("not att") == NULL);
  assertTrue(pXML2->GetTagName() != NULL);
  assertTrue(std::string(pXML2->GetTagName()) == tag1);
  assertTrue(pXML2->GetCharacterData() != NULL);
  assertTrue(std::string(pXML2->GetCharacterData()) == data1);
  assertTrue(pXML2->GetUseCData() == false);
  assertTrue(pXML2->GetComment() != NULL);
  assertTrue(std::string(pXML2->GetComment()) == comment1);

  soarxml::ElementXML::DeleteString(pStr);

  delete pXML2;
  delete pXML1;
}

void ElementXMLTest::testBinaryData() {
  soarxml::ElementXML* pXML1 = createXML1();
  soarxml::ElementXML* pXML2 = createXML2();
  soarxml::ElementXML* pXML4 = createXML4();
  soarxml::ElementXML* pXML5 = createXML5();

  pXML4->AddChild(pXML1);
  pXML4->AddChild(pXML2);
  pXML4->AddChild(pXML5);

  char* pStr = pXML4->GenerateXMLString(true);
  soarxml::ElementXML* pParsedXML =
      soarxml::ElementXML::ParseXMLFromString(pStr);

  assertTrue(pParsedXML != NULL);
  assertTrue(pParsedXML->GetNumberChildren() == 3);

  soarxml::ElementXML child0(NULL);
  soarxml::ElementXML const* pChild0 = &child0;
  assertTrue(pParsedXML->GetChild(&child0, 0));
  assertTrue(pChild0->GetTagName() != NULL);
  assertTrue(std::string(pChild0->GetTagName()) == tag1);
  assertTrue(pChild0->GetCharacterData() != NULL);
  assertTrue(std::string(pChild0->GetCharacterData()) == data1);
  assertTrue(pChild0->GetNumberAttributes() == 2);
  assertTrue(pChild0->GetNumberChildren() == 0);

  soarxml::ElementXML child1(NULL);
  soarxml::ElementXML const* pChild1 = &child1;
  assertTrue(pParsedXML->GetChild(&child1, 1));
  assertTrue(pChild1->GetTagName() != NULL);
  assertTrue(std::string(pChild1->GetTagName()) == tag2);
  assertTrue(pChild1->GetCharacterData() != NULL);
  assertTrue(std::string(pChild1->GetCharacterData()) == data2);
  assertTrue(pChild0->GetNumberChildren() == 0);
  assertTrue(pChild1->GetAttribute(att21.c_str()) != NULL);
  assertTrue(std::string(pChild1->GetAttribute(att21.c_str())) == val21);

  soarxml::ElementXML child2(NULL);
  soarxml::ElementXML const* pChild2 = &child2;
  assertTrue(pParsedXML->GetChild(&child2, 2));
  assertTrue(pChild2->IsCharacterDataBinary());

  const char* pBuffer = pChild2->GetCharacterData();
  int bufferLen = pChild2->GetCharacterDataLength();

  assertTrue(bufferLen == BUFFER_LENGTH);
  assertTrue(verifyBuffer(pBuffer));

  soarxml::ElementXML::DeleteString(pStr);

  delete pXML4;
  delete pParsedXML;
}

void ElementXMLTest::testEquals() {
  soarxml::ElementXML* element =
      soarxml::ElementXML::ParseXMLFromString("<sml><result>=</result></sml>");
  assertTrue_msg(soarxml::ElementXML::GetLastParseErrorDescription(),
                 element != 0);
  delete element;
}
